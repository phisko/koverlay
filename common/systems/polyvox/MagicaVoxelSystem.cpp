#include "MagicaVoxelSystem.hpp"

#include <unordered_map>

#include <PolyVox/RawVolume.h>
#include <PolyVox/CubicSurfaceExtractor.h>
#include <PolyVox/VolumeResampler.h>

#include "EntityManager.hpp"

#include "components/ModelLoaderComponent.hpp"
#include "components/ModelComponent.hpp"
#include "components/GraphicsComponent.hpp"
#include "components/PolyVoxComponent.hpp"
#include "components/DefaultShadowComponent.hpp"

#include "string.hpp"
#include "Export.hpp"
#include "file_extension.hpp"
#include "MagicaVoxel.hpp"

namespace kengine {
	namespace detailMagicaVoxel {
		static auto buildMesh(PolyVox::RawVolume<PolyVoxComponent::VertexData> & volume) {
			const auto encodedMesh = PolyVox::extractCubicMesh(&volume, volume.getEnclosingRegion());
			const auto mesh = PolyVox::decodeMesh(encodedMesh);
			return mesh;
		}
	}

	static float toNormalizedColor(unsigned char color) {
		return (float)color / 255.f;
	}

	using MeshType = decltype(detailMagicaVoxel::buildMesh(PolyVox::RawVolume<PolyVoxComponent::VertexData>{ {} }));

	struct MagicaVoxelModelComponent : not_serializable {
		MeshType mesh;
	};

	static bool idMatches(const char * s1, const char * s2) {
		return strncmp(s1, s2, 4) == 0;
	}

	template<typename T>
	static void readFromStream(T & header, std::istream & s, unsigned int size) {
		s.read((char *)&header, size);
		assert(s.gcount() == size);
	}

	template<typename T>
	static void readFromStream(T & header, std::istream & s) {
		readFromStream(header, s, sizeof(header));
	}

	static void checkHeader(std::istream & s) {
		MagicaVoxel::FileHeader header;
		readFromStream(header, s);
		assert(idMatches(header.id, "VOX "));
		assert(header.versionNumber == 150);
	}

	static auto release(Entity::ID id, EntityManager & em) {
		return [&em, id] {
			auto & e = em.getEntity(id);
			auto & model = e.get<MagicaVoxelModelComponent>();
			model.mesh.clear();
			e.detach<MagicaVoxelModelComponent>();
		};
	}

	static auto extractData(Entity::ID id, EntityManager & em) {
		return [&em, id] {
			auto & e = em.getEntity(id);

			ModelLoaderComponent::ModelData ret; {
				ModelLoaderComponent::ModelData::MeshData meshData; {
					const auto & model = e.get<MagicaVoxelModelComponent>();
					meshData.vertices = { model.mesh.getNoOfVertices(), sizeof(pmeta_typeof(model.mesh)::VertexType), model.mesh.getRawVertexData() };
					meshData.indices = { model.mesh.getNoOfIndices(), sizeof(pmeta_typeof(model.mesh)::IndexType), model.mesh.getRawIndexData() };
					meshData.indexType = sizeof(pmeta_typeof(model.mesh)::IndexType) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT;
				}
				ret.meshes.push_back(meshData);
			}
			return ret;
		};
	}

	static bool loadVoxFile(Entity & e) {
		const auto & f = e.get<ModelComponent>().file.c_str();
		if (strcmp(putils::file_extension(f), "vox") != 0)
			return false;

#ifndef NDEBUG
		std::cout << "[MagicaVoxel] Loading " << f << "...";
#endif
		std::ifstream stream(f, std::ios::binary);
		assert(stream);
		checkHeader(stream);

		MagicaVoxel::ChunkHeader main;
		readFromStream(main, stream);
		assert(idMatches(main.id, "MAIN"));

		MagicaVoxel::ChunkHeader first;
		readFromStream(first, stream);
		assert(idMatches(first.id, "SIZE")); // "PACK" not yet supported
		assert(first.childrenBytes == 0);
		assert(first.contentBytes == sizeof(MagicaVoxel::ChunkContent::Size));

		MagicaVoxel::ChunkContent::Size size;
		readFromStream(size, stream);

		PolyVox::RawVolume<PolyVoxComponent::VertexData> volume(PolyVox::Region{ { 0, 0, 0, }, { size.x, size.z, size.y } });

		MagicaVoxel::ChunkHeader voxelsHeader;
		readFromStream(voxelsHeader, stream);
		assert(idMatches(voxelsHeader.id, "XYZI"));

		MagicaVoxel::ChunkContent::RGBA rgba;

		MagicaVoxel::ChunkContent::XYZI xyzi;
		readFromStream(xyzi, stream);
		assert(voxelsHeader.contentBytes == sizeof(xyzi) + xyzi.numVoxels * sizeof(int));

		for (int i = 0; i < xyzi.numVoxels; ++i) {
			MagicaVoxel::ChunkContent::XYZI::Voxel voxel;
			readFromStream(voxel, stream);

			const auto & color = rgba.palette[voxel.colorIndex];
			const PolyVoxComponent::VertexData voxelValue{ { toNormalizedColor(color.r), toNormalizedColor(color.g), toNormalizedColor(color.b) } };
			volume.setVoxel(voxel.x, voxel.z, voxel.y, voxelValue);
		}

		e.attach<MagicaVoxelModelComponent>().mesh = detailMagicaVoxel::buildMesh(volume);

		auto & comp = e.get<ModelComponent>();
		auto & box = comp.boundingBox;
		auto & offset = box.topLeft;

		offset.x += size.x / 2.f * box.size.x;
		offset.z += size.y / 2.f * box.size.z;

#ifndef NDEBUG
		std::cout << " Done.\n";
#endif
		return true;
	}

	MagicaVoxelSystem::MagicaVoxelSystem(EntityManager & em) : System(em), _em(em) {
	}

	void MagicaVoxelSystem::loadModel(Entity & e) {
		if (!loadVoxFile(e))
			return;

		e += ModelLoaderComponent{
			extractData(e.id, _em),
			release(e.id, _em),
			[]() { putils::gl::setPolyvoxVertexType<MeshType::VertexType>(); }
		};
	}

	void MagicaVoxelSystem::setModel(Entity & e) {
		auto & graphics = e.get<GraphicsComponent>();

		auto & layer = graphics.getLayer("main");
		if (strcmp(putils::file_extension(layer.appearance.c_str()), "vox") != 0)
			return;

		e += PolyVoxObjectComponent{};
		e += DefaultShadowComponent{};

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


	void MagicaVoxelSystem::handle(packets::RegisterEntity p) {
		if (p.e.has<ModelComponent>())
			loadModel(p.e);
		else if (p.e.has<GraphicsComponent>())
			setModel(p.e);
	}
}
