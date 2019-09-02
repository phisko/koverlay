# importing the requests library 
import threading
import requests

# api-endpoint 
URL = "https://opensky-network.org/api/states/all"

# defining a params dict for the parameters to be sent to the API 
PARAMS = {'lamin': 48.724017, 'lomin': 2.356484, 'lamax': 48.775232, 'lomax': 2.539622} 

# sending get request and saving the response as response object 
r = requests.get(url = URL, params = PARAMS)

closestPlane = str(r.json())