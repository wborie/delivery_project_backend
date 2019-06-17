import requests
import json

# res = requests.post("http://localhost:4000/testEndpoint", json={'first':1})

# roads = ["rd1", "rd2", "rd3", "rd4", "rd5", "rd6", "rd7", "rd8"]

roadSectors = [
	{"roadName": "rd1", "roadSectorId": "1", "roadSectorLength": 10, "addressMinNumber": 1, "addressMaxNumber": 10},
	{"roadName": "rd1", "roadSectorId": "2", "roadSectorLength": 10, "addressMinNumber": 11, "addressMaxNumber": 20},
	{"roadName": "rd2", "roadSectorId": "1", "roadSectorLength": 15.73133, "addressMinNumber": 1, "addressMaxNumber": 15},
	{"roadName": "rd3", "roadSectorId": "1", "roadSectorLength": 10, "addressMinNumber": 1, "addressMaxNumber": 10},
	{"roadName": "rd3", "roadSectorId": "2", "roadSectorLength": 10, "addressMinNumber": 11, "addressMaxNumber": 20},
	{"roadName": "rd4", "roadSectorId": "1", "roadSectorLength": 5, "addressMinNumber": 1, "addressMaxNumber": 5},
	{"roadName": "rd4", "roadSectorId": "2", "roadSectorLength": 5, "addressMinNumber": 6, "addressMaxNumber": 10},
	{"roadName": "rd4", "roadSectorId": "3", "roadSectorLength": 3, "addressMinNumber": 11, "addressMaxNumber": 13},
	{"roadName": "rd4", "roadSectorId": "4", "roadSectorLength": 2, "addressMinNumber": 14, "addressMaxNumber": 15},
	{"roadName": "rd4", "roadSectorId": "5", "roadSectorLength": 5, "addressMinNumber": 15, "addressMaxNumber": 20},
	{"roadName": "rd5", "roadSectorId": "1", "roadSectorLength": 5, "addressMinNumber": 1, "addressMaxNumber": 5},
	{"roadName": "rd6", "roadSectorId": "1", "roadSectorLength": 15.73133, "addressMinNumber": 1, "addressMaxNumber": 15},
	{"roadName": "rd7", "roadSectorId": "1", "roadSectorLength": 15.73133, "addressMinNumber": 1, "addressMaxNumber": 15},
	{"roadName": "rd8", "roadSectorId": "1", "roadSectorLength": 5, "addressMinNumber": 1, "addressMaxNumber": 5}
]

intersections = [
	{"roadSectors": [{"roadName": "rd8", "roadSectorId": "1"}], "x": 0, "y": 0, "isEndPoint": True}, # A
	{"roadSectors": [{"roadName": "rd1", "roadSectorId": "1"}, 
						{"roadName": "rd2", "roadSectorId": "1"}, 
						{"roadName": "rd8", "roadSectorId": "1"}
					], "x": 3.53553, "y": 3.53553, "isEndPoint": False}, # B
	{"roadSectors": [{"roadName": "rd1", "roadSectorId": "1"}, 
						{"roadName": "rd1", "roadSectorId": "2"}, 
						{"roadName": "rd6", "roadSectorId": "1"}
					], "x": 3.53553, "y": 13.53553, "isEndPoint": False}, # C
	{"roadSectors": [{"roadName": "rd1", "roadSectorId": "2"}, 
						{"roadName": "rd4", "roadSectorId": "1"}, 
						{"roadName": "rd7", "roadSectorId": "1"}
					], "x": 3.53553, "y": 23.53553, "isEndPoint": False}, # D
	{"roadSectors": [{"roadName": "rd4", "roadSectorId": "1"}, 
						{"roadName": "rd4", "roadSectorId": "2"}
					], "x": 7.07106, "y": 27.07106, "isEndPoint": False}, # E
	{"roadSectors": [{"roadName": "rd4", "roadSectorId": "2"}, 
						{"roadName": "rd4", "roadSectorId": "3"}
					], "x": 11.40119, "y": 29.57106, "isEndPoint": False}, # F
	{"roadSectors": [{"roadName": "rd4", "roadSectorId": "3"}, 
						{"roadName": "rd4", "roadSectorId": "4"}, 
						{"roadName": "rd5", "roadSectorId": "1"}
					], "x": 13.56626, "y": 28.32106, "isEndPoint": False}, # G
	{"roadSectors": [{"roadName": "rd5", "roadSectorId": "1"}], "x": 17.10179, "y": 31.85659, "isEndPoint": True}, # H
	{"roadSectors": [{"roadName": "rd4", "roadSectorId": "4"}, 
						{"roadName": "rd4", "roadSectorId": "5"}
					], "x": 15.73133, "y": 27.07106, "isEndPoint": False}, # I
	{"roadSectors": [{"roadName": "rd4", "roadSectorId": "5"}, 
						{"roadName": "rd3", "roadSectorId": "2"}, 
						{"roadName": "rd7", "roadSectorId": "1"}
					], "x": 19.26686, "y": 23.53553, "isEndPoint": False}, # J
	{"roadSectors": [{"roadName": "rd3", "roadSectorId": "2"}, 
						{"roadName": "rd6", "roadSectorId": "1"}, 
						{"roadName": "rd3", "roadSectorId": "1"}
					], "x": 19.26686, "y": 13.53553, "isEndPoint": False}, # K
	{"roadSectors": [{"roadName": "rd3", "roadSectorId": "1"}, 
						{"roadName": "rd2", "roadSectorId": "1"}
					], "x": 19.26686, "y": 3.53553, "isEndPoint": False} # L
]

locations = [
	{"name": "res1", "type": "restaurant", "roadName": "rd1", "addressNumber": 12, "x": 3.53553, "y": 15.53553},
	{"name": "res2", "type": "restaurant", "roadName": "rd1", "addressNumber": 8, "x": 3.53553, "y": 11.53553},
	{"name": "res3", "type": "restaurant", "roadName": "rd2", "addressNumber": 5, "x": 8.53553, "y": 3.53553},
	{"name": "h1", "type": "house", "roadName": "rd3", "addressNumber": 3, "x": 19.26686, "y": 6.53553},
	{"name": "h2", "type": "house", "roadName": "rd3", "addressNumber": 15, "x": 19.26686, "y": 18.53553},
	{"name": "h3", "type": "house", "roadName": "rd5", "addressNumber": 2, "x": 14.98047, "y": 29.73527},
]

roadSectorSuccessCounter = 0
for roadSector in roadSectors:
	res = requests.post("http://localhost:4000/roadSector", json=roadSector)
	if (res.status_code == 400):
		print(res.status_code, res.json())
	else:
		roadSectorSuccessCounter += 1
print("# of successful RoadSector Insertions:", roadSectorSuccessCounter)
	
intersectionSuccessCounter = 0
for intersection in intersections:
	res = requests.post("http://localhost:4000/intersection", json=intersection)
	if (res.status_code == 400):
		print(res.status_code, res.json())
	else:
		intersectionSuccessCounter += 1
print("# of successful Intersection Insertions:", intersectionSuccessCounter)

locationSuccessCounter = 0
for location in locations:
	res = requests.post("http://localhost:4000/location", json=location)
	if (res.status_code == 400):
		print(res.status_code, res.json())
	else:
		locationSuccessCounter += 1
print("# of successful Location Insertions:", locationSuccessCounter)

# print(res.status_code, res.reason)
# print(res.json())

# res = requests.post("http://localhost:4000/testEndpoint", json={'first':1})
