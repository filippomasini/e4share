import sys
import random
import math

if(len(sys.argv) != 7):
	print("required arguments: width height stations cars trips maxtime", file=sys.stderr)
	exit()

width = int(sys.argv[1])
height = int(sys.argv[2])
vertexCount = width * height
vertices = range(0, vertexCount)
edgeCount = (width - 1) * height + (height - 1) * width
stationCount = int(sys.argv[3])
if(stationCount > vertexCount):
	stationCount = vertexCount
carCount = int(sys.argv[4])
tripCount = int(sys.argv[5])
maxtime = int(sys.argv[6])

edges = list()
for y in range(0, height):
	for x in range(0, width):
		vertex = y * width + x
		if(x < (width - 1)):
			edge = (vertex, vertex + 1, 1)
			edges.append(edge)
		if(y < (height - 1)):
			edge = (vertex, vertex + width, 1)
			edges.append(edge)

stations = list()
selectedLocations = random.sample(vertices, stationCount)
for loc in selectedLocations:
	cost = random.randrange(1, 100)
	costPerSlot = random.randrange(1, 100)
	capacity = random.randrange(5, 11)
	stations.append((loc, cost, costPerSlot, capacity))
stations = sorted(stations, key = lambda station: station[0])

trips = list()
for i in range(0, tripCount):
	origin = random.choice(vertices)
	destination = random.choice(vertices)
	begintime = random.randrange(0, maxtime)
	endtime = random.randrange(begintime + 1, maxtime + 1)
	batteryconsumption = math.ceil(random.uniform(0.1, 0.5) * 100) / 100
	profit = 1
	trips.append((origin, destination, begintime, endtime, batteryconsumption, profit))

# print
print(vertexCount, edgeCount, stationCount, carCount, tripCount)
for edge in edges:
	for part in edge:
		print(part, end = ' ')
	print()
for station in stations:
	for part in station:
		print(part, end = ' ')
	print()
for trip in trips:
	for part in trip:
		print(part, end = ' ')
	print()
