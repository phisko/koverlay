#include "AssimpSystem.hpp"
#include "EntityManager.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "file_extension.hpp"
#include "imgui.h"

#include "AssImpShader.hpp"
#include "AssImpShadowMap.hpp"
#include "AssImpShadowCube.hpp"

#include "components/GraphicsComponent.hpp"
#include "components/ModelLoaderComponent.hpp"
#include "components/TextureLoaderComponent.hpp"
#include "components/TextureModelComponent.hpp"
#include "components/ModelComponent.hpp"

#include "components/AnimationComponent.hpp"
#include "components/SkeletonComponent.hpp"

#include "components/ShaderComponent.hpp"
#include "components/ImGuiComponent.hpp"

#include "packets/GBuffer.hpp"

#include "AssImpHelper.hpp"

static kengine::EntityManager * g_em = nullptr;

static Assimp::Importer g_importer;

namespace kengine {
	namespace AssImp {
		struct AssImpModelComponent : kengine::not_serializable {
			struct Mesh {
				struct Vertex {
					float position[3];
					float normal[3];
					float texCoords[2];
					float boneWeights[KENGINE_ASSIMP_BONE_INFO_PER_VERTEX] = { 0.f };
					unsigned int boneIDs[KENGINE_ASSIMP_BONE_INFO_PER_VERTEX] = { 0 };

					pmeta_get_attributes(
						pmeta_reflectible_attribute(&Vertex::position),
						pmeta_reflectible_attribute(&Vertex::normal),
						pmeta_reflectible_attribute(&Vertex::texCoords),

						pmeta_reflectible_attribute(&Vertex::boneWeights),
						pmeta_reflectible_attribute(&Vertex::boneIDs)
					);
				};

				std::vector<Vertex> vertices;
				std::vector<unsigned int> indices;
			};

			Assimp::Importer importer;
			putils::vector<Assimp::Importer, KENGINE_MAX_ANIMATION_FILES> animImporters;
			std::vector<Mesh> meshes;
		};

		struct AssImpSkeletonComponent : kengine::not_serializable {
			struct Mesh {
				struct Bone {
					aiNode * node = nullptr;
					std::vector<const aiNodeAnim *> animNodes;
					glm::mat4 offset;
				};
				putils::vector<Bone, KENGINE_SKELETON_MAX_BONES> bones;
			};

			aiNode * rootNode;
			std::vector<Mesh> meshes;
			glm::mat4 globalInverseTransform;

			pmeta_get_class_name(AssImpSkeletonComponent);
		};

		static aiMatrix4x4 toAiMat(const glm::mat4 & mat) {
			return aiMatrix4x4(mat[0][0], mat[1][0], mat[2][0], mat[3][0],
				mat[0][1], mat[1][1], mat[2][1], mat[3][1],
				mat[0][2], mat[1][2], mat[2][2], mat[3][2],
				mat[0][3], mat[1][3], mat[2][3], mat[3][3]);
		}

		static glm::mat4 toglm(const aiMatrix4x4 & mat) {
			return glm::make_mat4(&mat.a1);
		}

		static glm::mat4 toglmWeird(const aiMatrix4x4 & mat) {
			return glm::transpose(toglm(mat));
		}

		static glm::vec3 toglm(const aiVector3D & vec) { return { vec.x, vec.y, vec.z }; }

		static glm::quat toglm(const aiQuaternion & quat) { return { quat.w, quat.x, quat.y, quat.z }; }

		template<typename T>
		static unsigned int findPreviousIndex(T * arr, unsigned int size, float time) {
			for (unsigned i = 0; i < size - 1; ++i) {
				if (time < (float)arr[i + 1].mTime)
					return i;
			}
			return 0;
		}

		template<typename T, typename Func>
		static auto calculateInterpolatedValue(T * arr, unsigned int size, float time, Func func) {
			if (size == 1)
				return toglm(arr[0].mValue);

			const auto index = findPreviousIndex(arr, size, time);
			const auto & value = arr[index];
			const auto & nextValue = arr[index + 1];

			const auto deltaTime = (float)nextValue.mTime - (float)value.mTime;
			const auto factor = (time - (float)value.mTime) / (float)deltaTime;

			const auto startValue = toglm(value.mValue);
			const auto endValue = toglm(nextValue.mValue);

			return func(startValue, endValue, factor);
		}

		static glm::vec3 calculateInterpolatedPosition(const AssImpSkeletonComponent::Mesh::Bone & bone, float time, size_t currentAnim) {
			return calculateInterpolatedValue(bone.animNodes[currentAnim]->mPositionKeys, bone.animNodes[currentAnim]->mNumPositionKeys, time, [](const glm::vec3 & v1, const glm::vec3 & v2, float f) { return glm::mix(v1, v2, f); });
		}

		static glm::quat calculateInterpolatedRotation(const AssImpSkeletonComponent::Mesh::Bone & bone, float time, size_t currentAnim) {
			return calculateInterpolatedValue(bone.animNodes[currentAnim]->mRotationKeys, bone.animNodes[currentAnim]->mNumRotationKeys, time, glm::slerp<float, glm::defaultp>);
		}

		static glm::vec3 calculateInterpolatedScale(const AssImpSkeletonComponent::Mesh::Bone & bone, float time, size_t currentAnim) {
			return calculateInterpolatedValue(bone.animNodes[currentAnim]->mScalingKeys, bone.animNodes[currentAnim]->mNumScalingKeys, time, [](const glm::vec3 & v1, const glm::vec3 & v2, float f) { return glm::mix(v1, v2, f); });
		}

		static void updateBoneMats(const aiNode * node, float time, size_t currentAnim, const AssImpSkeletonComponent & assimp, SkeletonComponent & comp, const glm::mat4 & parentTransform) {
			bool firstCalc = true;
			glm::mat4 totalTransform = parentTransform * toglmWeird(node->mTransformation);

			for (unsigned int i = 0; i < assimp.meshes.size(); ++i) {
				const auto & input = assimp.meshes[i];
				auto & output = comp.meshes[i];

				size_t boneIndex = 0;
				for (const auto & bone : input.bones) {
					if (bone.node == node)
						break;
					++boneIndex;
				}

				if (boneIndex != input.bones.size()) {
					const auto & modelBone = input.bones[boneIndex];

					if (firstCalc) {
						glm::mat4 mat(1.f);
						if (modelBone.animNodes[currentAnim] != nullptr) {
							const auto pos = calculateInterpolatedPosition(modelBone, time, currentAnim);
							const auto rot = calculateInterpolatedRotation(modelBone, time, currentAnim);
							const auto scale = calculateInterpolatedScale(modelBone, time, currentAnim);

							mat = glm::translate(mat, pos);
							mat *= glm::mat4_cast(rot);
							mat = glm::scale(mat, scale);
						}
						totalTransform = parentTransform * mat;

						firstCalc = false;
					}

					output.boneMatsMeshSpace[boneIndex] = totalTransform;
					output.boneMatsBoneSpace[boneIndex] = totalTransform * modelBone.offset;
				}
			}

			for (size_t i = 0; i < node->mNumChildren; ++i)
				updateBoneMats(node->mChildren[i], time, currentAnim, assimp, comp, totalTransform);
		}

		static void loadMaterialTextures(std::vector<Entity::ID> & textures, const char * directory, const aiMaterial * mat, aiTextureType type) {
			for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i) {
				aiString path;
				mat->GetTexture(type, i, &path);

				const putils::string<KENGINE_TEXTURE_PATH_MAX_LENGTH> fullPath("%s/%s", directory, path.C_Str());

				Entity::ID modelID = Entity::INVALID_ID;
				for (const auto &[e, model, tex] : g_em->getEntities<ModelComponent, TextureModelComponent>())
					if (model.file == fullPath) {
						modelID = e.id;
						break;
					}

				if (modelID == Entity::INVALID_ID) {
					*g_em += [&](Entity & e) {
						modelID = e.id;

						auto & comp = e.attach<ModelComponent>();
						comp.file = fullPath.c_str();

						TextureLoaderComponent textureLoader; {
							textureLoader.textureID = &e.attach<TextureModelComponent>().texture;
							textureLoader.data = stbi_load(fullPath.c_str(), &textureLoader.width, &textureLoader.height, &textureLoader.components, 0);
							assert(textureLoader.data != nullptr);
							textureLoader.free = stbi_image_free;
						} e += textureLoader;
					};
				}

				textures.push_back(modelID);
			}
		}

		static AssImpModelComponent::Mesh processMesh(const aiMesh * mesh) {
			AssImpModelComponent::Mesh ret;

			for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
				AssImpModelComponent::Mesh::Vertex vertex;

				vertex.position[0] = mesh->mVertices[i].x;
				vertex.position[1] = mesh->mVertices[i].y;
				vertex.position[2] = mesh->mVertices[i].z;

				vertex.normal[0] = mesh->mNormals[i].x;
				vertex.normal[1] = mesh->mNormals[i].y;
				vertex.normal[2] = mesh->mNormals[i].z;

				if (mesh->mTextureCoords[0] != nullptr) {
					vertex.texCoords[0] = mesh->mTextureCoords[0][i].x;
					vertex.texCoords[1] = mesh->mTextureCoords[0][i].y;
				}
				else {
					vertex.texCoords[0] = 0.f;
					vertex.texCoords[1] = 0.f;
				}

				ret.vertices.push_back(vertex);
			}

			// for each bone
			for (unsigned int i = 0; i < mesh->mNumBones; ++i) {
				const auto bone = mesh->mBones[i];
				// for each weight (vertex it has an influence on)
				for (unsigned int j = 0; j < bone->mNumWeights; ++j) {
					const auto & weight = bone->mWeights[j];
					auto & vertex = ret.vertices[weight.mVertexId];

					// add this bone to the vertex
					bool found = false;
					for (unsigned int k = 0; k < KENGINE_ASSIMP_BONE_INFO_PER_VERTEX; ++k)
						if (vertex.boneWeights[k] == 0.f) {
							found = true;
							vertex.boneWeights[k] = weight.mWeight;
							vertex.boneIDs[k] = i;
							break;
						}
					assert(found); // too many bones have info for a single vertex
					if (!found) {
						float smallestWeight = FLT_MAX;
						unsigned int smallestIndex = 0;
						for (unsigned int k = 0; k < KENGINE_ASSIMP_BONE_INFO_PER_VERTEX; ++k)
							if (vertex.boneWeights[k] < smallestWeight) {
								smallestWeight = vertex.boneWeights[k];
								smallestIndex = k;
							}
						if (weight.mWeight > smallestWeight)
							vertex.boneWeights[smallestIndex] = weight.mWeight;
					}
				}
			}

			// For models with no skeleton
			for (auto & vertex : ret.vertices)
				if (vertex.boneWeights[0] == 0.f) {
					vertex.boneWeights[0] = 1.f;
					vertex.boneIDs[0] = 0;
				}

			for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
				for (unsigned int j = 0; j < mesh->mFaces[i].mNumIndices; ++j)
					ret.indices.push_back(mesh->mFaces[i].mIndices[j]);
			return ret;
		}

		static AssImpTexturesModelComponent::MeshTextures processMeshTextures(const char * directory, const aiMesh * mesh, const aiScene * scene) {
			AssImpTexturesModelComponent::MeshTextures meshTextures;
			if (mesh->mMaterialIndex >= 0) {
				const auto material = scene->mMaterials[mesh->mMaterialIndex];
				loadMaterialTextures(meshTextures.diffuse, directory, material, aiTextureType_DIFFUSE);
				loadMaterialTextures(meshTextures.specular, directory, material, aiTextureType_SPECULAR);

				aiColor3D color{ 0.f, 0.f, 0.f };
				material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
				meshTextures.diffuseColor = { color.r, color.g, color.b };
				material->Get(AI_MATKEY_COLOR_SPECULAR, color);
				meshTextures.specularColor = { color.r, color.g, color.b };
			}
			else
				assert(false);
			return meshTextures;
		}

		static void processNode(AssImpModelComponent & modelData, AssImpTexturesModelComponent & textures, const char * directory, const aiNode * node, const aiScene * scene, bool firstLoad) {
			for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
				const aiMesh * mesh = scene->mMeshes[node->mMeshes[i]];
				if (firstLoad)
					modelData.meshes.push_back(processMesh(mesh));
				textures.meshes.push_back(processMeshTextures(directory, mesh, scene));
			}

			for (unsigned int i = 0; i < node->mNumChildren; ++i)
				processNode(modelData, textures, directory, node->mChildren[i], scene, firstLoad);
		}

		static void addNode(std::vector<aiNode *> & allNodes, aiNode * node) {
			allNodes.push_back(node);
			for (unsigned int i = 0; i < node->mNumChildren; ++i)
				addNode(allNodes, node->mChildren[i]);
		}

		static const AssImpSkeletonComponent::Mesh::Bone * findBone(const ModelSkeletonComponent::Mesh & model, const AssImpSkeletonComponent::Mesh & skeleton, const char * name) {
			for (unsigned int i = 0; i < skeleton.bones.size(); ++i)
				if (model.boneNames[i] == name)
					return &skeleton.bones[i];
			return nullptr;
		}

		static aiNode * findNode(const std::vector<aiNode *> & allNodes, const char * name) {
			for (const auto node : allNodes)
				if (strcmp(node->mName.data, name) == 0)
					return node;
			assert(false);
			return nullptr;
		}

		static aiNodeAnim * findNodeAnim(const std::vector<aiNodeAnim *> & allNodes, const char * name) {
			for (const auto node : allNodes)
				if (strcmp(node->mNodeName.data, name) == 0)
					return node;
			return nullptr;
		}

		static void addAnim(const char * animFile, aiAnimation * aiAnim, const ModelSkeletonComponent & model, AssImpSkeletonComponent & skeleton, AnimListComponent & animList) {
			AnimListComponent::Anim anim;
			anim.name.set("%s/%s", animFile, aiAnim->mName.data);
			anim.ticksPerSecond = (float)(aiAnim->mTicksPerSecond != 0 ? aiAnim->mTicksPerSecond : 25.0);
			anim.totalTime = (float)aiAnim->mDuration / anim.ticksPerSecond;
			animList.allAnims.push_back(anim);

			std::vector<aiNodeAnim *> allNodeAnims;
			for (unsigned int i = 0; i < aiAnim->mNumChannels; ++i)
				allNodeAnims.push_back(aiAnim->mChannels[i]);

			for (unsigned int i = 0; i < skeleton.meshes.size(); ++i) {
				auto & mesh = skeleton.meshes[i];
				for (unsigned int j = 0; j < mesh.bones.size(); ++j) {
					auto & bone = mesh.bones[j];
					const auto & boneName = model.meshes[i].boneNames[j];
					bone.animNodes.push_back(findNodeAnim(allNodeAnims, boneName.c_str()));
				}
			}
		}

		static auto extractData(Entity::ID id, EntityManager & em) {
			return [id, &em] {
				auto & e = em.getEntity(id);
				auto & model = e.get<AssImpModelComponent>();

				ModelLoaderComponent::ModelData ret;
				for (const auto & mesh : model.meshes) {
					decltype(ret)::MeshData meshData;
					meshData.vertices = { mesh.vertices.size(), sizeof(AssImpModelComponent::Mesh::Vertex), mesh.vertices.data() };
					meshData.indices = { mesh.indices.size(), sizeof(mesh.indices[0]), mesh.indices.data() };
					meshData.indexType = GL_UNSIGNED_INT;
					ret.meshes.push_back(meshData);
				}

				return ret;
			};
		}

		static auto release(Entity::ID id, EntityManager & em) {
			return [id, &em] {
#if 0 // Disabled because scene is still used for animation at this point
				auto & e = em.getEntity(id);

				auto & model = e.attach<AssImpModelComponent>(); // previous attach hasn't been processed yet, so `get` would assert
				model.importer.FreeScene();
				for (auto & importer : model.animImporters)
					importer.FreeScene();
				e.detach<AssImpModelComponent>();
#endif
			};
		}

		static bool loadFile(Entity & e) {
			const auto & f = e.get<ModelComponent>().file.c_str();
			if (!g_importer.IsExtensionSupported(putils::file_extension(f)))
				return false;

#ifndef NDEBUG
			std::cout << "[AssImp] Loading " << f << "...";
#endif

			auto & model = e.attach<AssImpModelComponent>();

			auto & textures = e.attach<AssImpTexturesModelComponent>();
			textures.meshes.clear();
			auto & skeletonNames = e.attach<ModelSkeletonComponent>();
			skeletonNames.meshes.clear();
			auto & skeleton = e.attach<AssImpSkeletonComponent>();
			skeleton.meshes.clear();
			auto & animList = e.attach<AnimListComponent>();
			animList.allAnims.clear();

			bool firstLoad = false;

			if (model.importer.GetScene() == nullptr) {
				const auto scene = model.importer.ReadFile(f, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals /*| aiProcess_OptimizeMeshes*/ | aiProcess_JoinIdenticalVertices);
				if (scene == nullptr || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || scene->mRootNode == nullptr) {
					std::cerr << model.importer.GetErrorString() << '\n';
					assert(false);
				}
				firstLoad = true;
			}
			const auto scene = model.importer.GetScene();

			const auto dir = putils::get_directory<64>(f);
			processNode(model, textures, dir.c_str(), scene->mRootNode, scene, firstLoad);

			std::vector<aiNode *> allNodes;
			addNode(allNodes, scene->mRootNode);
			skeleton.rootNode = scene->mRootNode;

			for (unsigned int i = 0; i < scene->mNumMeshes; ++i) {
				const auto mesh = scene->mMeshes[i];

				ModelSkeletonComponent::Mesh meshNames;
				AssImpSkeletonComponent::Mesh meshBones;
				for (unsigned int i = 0; i < mesh->mNumBones; ++i) {
					const auto aiBone = mesh->mBones[i];
					const auto name = aiBone->mName.data;

					AssImpSkeletonComponent::Mesh::Bone bone;
					bone.node = findNode(allNodes, name);
					bone.offset = toglmWeird(aiBone->mOffsetMatrix);
					meshBones.bones.push_back(bone);

					meshNames.boneNames.push_back(name);
				}
				skeleton.meshes.emplace_back(std::move(meshBones));
				skeletonNames.meshes.emplace_back(std::move(meshNames));
			}

			skeleton.globalInverseTransform = glm::inverse(toglmWeird(scene->mRootNode->mTransformation));

			for (unsigned int i = 0; i < scene->mNumAnimations; ++i)
				addAnim(f, scene->mAnimations[i], skeletonNames, skeleton, animList);

			if (e.has<AnimFilesComponent>()) {
				const auto & animFiles = e.get<AnimFilesComponent>();
				model.animImporters.resize(animFiles.files.size());

				size_t i = 0;
				for (const auto & f : animFiles.files) {
					auto & importer = model.animImporters[i];

					const auto scene = importer.ReadFile(f.c_str(), aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals /*| aiProcess_OptimizeMeshes*/ | aiProcess_JoinIdenticalVertices);
					if (scene == nullptr || scene->mRootNode == nullptr) {
						std::cerr << '\n' << importer.GetErrorString() << '\n';
						assert(false);
					}

					for (unsigned int i = 0; i < scene->mNumAnimations; ++i)
						addAnim(f.c_str(), scene->mAnimations[i], skeletonNames, skeleton, animList);

					++i;
				}
			}

#ifndef NDEBUG
			std::cout << "Done\n";
#endif
			return true;
		}
	}

	AssImpSystem::AssImpSystem(EntityManager & em) : System(em), _em(em) {
		g_em = &em;
		onLoad("");
	}

	void AssImpSystem::onLoad(const char *) noexcept {
		_em += [this](Entity & e) {
			e += makeGBufferShaderComponent<AssImpShader>(_em);
		};

		_em += [this](Entity & e) {
			e += makeLightingShaderComponent<AssImpShadowMap>(_em);
			e += ShadowMapShaderComponent{};
		};

		_em += [&](Entity & e) {
			e += makeLightingShaderComponent<AssImpShadowCube>(_em);
			e += ShadowCubeShaderComponent{};
		};
	}

	void AssImpSystem::loadModel(Entity & e) {
		if (!AssImp::loadFile(e))
			return;

		e += ModelLoaderComponent{
			AssImp::extractData(e.id, _em),
			AssImp::release(e.id, _em),
			[]() { putils::gl::setVertexType<AssImp::AssImpModelComponent::Mesh::Vertex>(); }
		};
	}

	void AssImpSystem::setModel(Entity & e) {
		auto & graphics = e.get<GraphicsComponent>();

		auto & layer = graphics.getLayer("main");
		if (!g_importer.IsExtensionSupported(putils::file_extension(layer.appearance.c_str())))
			return;

		e += AssImpObjectComponent{};
		e += SkeletonComponent{};

		layer.model = Entity::INVALID_ID;
		for (const auto &[e, model] : _em.getEntities<ModelComponent>())
			if (model.file == layer.appearance) {
				layer.model = e.id;
				return;
			}

		_em += [&](Entity & e) {
			e += ModelComponent{ layer.appearance.c_str() };
			layer.model = e.id;
		};
	}

	void AssImpSystem::handle(packets::RegisterEntity p) {
		if (p.e.has<ModelComponent>())
			loadModel(p.e);
		else if (p.e.has<GraphicsComponent>())
			setModel(p.e);
	}

	void AssImpSystem::execute() {
		const auto deltaTime = time.getDeltaTime().count();

		for (auto &[e, graphics, skeleton, anim] : _em.getEntities<GraphicsComponent, SkeletonComponent, AnimationComponent>())
		_em.runTask([&] {
			const auto & layer = graphics.getLayer("main");
			if (layer.model == Entity::INVALID_ID)
				return;

			auto & modelEntity = _em.getEntity(layer.model);
			if (!modelEntity.has<ModelComponent>() || !modelEntity.has<AssImp::AssImpSkeletonComponent>())
				return;

			const auto & animList = modelEntity.get<AnimListComponent>();

			if (anim.currentAnim >= animList.allAnims.size())
				return;
			const auto & currentAnim = animList.allAnims[anim.currentAnim];

			auto & assimp = modelEntity.get<AssImp::AssImpSkeletonComponent>();

			if (skeleton.meshes.empty())
				skeleton.meshes.resize(assimp.meshes.size());

			AssImp::updateBoneMats(assimp.rootNode, anim.currentTime * currentAnim.ticksPerSecond, anim.currentAnim, assimp, skeleton, glm::mat4(1.f));

			anim.currentTime += deltaTime * anim.speed;
			anim.currentTime = fmodf(anim.currentTime, currentAnim.totalTime);
		});
		_em.completeTasks();
	}
}