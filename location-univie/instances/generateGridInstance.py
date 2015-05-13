import sys
import random
import math
import networkx

def writeInstance(width, height, stationCount, maxCapacity, tripCount, minBattery, maxBattery, uniformProfit, maxTime, index):
	if(uniformProfit):
		profitStr = "uniform"
	else:
		profitStr = "nonuniform"
	filename = "%ix%i-s%i-cap%i-k%i-b%i-%i-t%i-%i.loc" % (width, height, stationCount, maxCapacity, tripCount, minBattery, maxBattery, maxTime, index)
	# parameters for calculating a trip's profit
	alpha = 2
	gamma = 1
	# log random "seed"
	randomstate = random.getstate()
	
	#print(filename)
	graph = networkx.Graph()
	vertexCount = width * height
	graph.add_nodes_from(range(0, vertexCount))
	#vertices = range(0, vertexCount)
	edgeCount = (width - 1) * height + (height - 1) * width
	if(stationCount > vertexCount):
		stationCount = vertexCount

	edges = list()
	for y in range(0, height):
		for x in range(0, width):
			vertex = y * width + x
			if(x < (width - 1)):
				distance = random.randrange(1, 6)
				graph.add_edge(vertex, vertex + 1, distance=distance)
				edge = (vertex, vertex + 1, distance)
				edges.append(edge)
			if(y < (height - 1)):
				distance = random.randrange(1, 6)
				graph.add_edge(vertex, vertex + width, distance=distance)
				edge = (vertex, vertex + width, distance)
				edges.append(edge)

	stations = list()
	selectedLocations = random.sample(graph.nodes(), stationCount)
	for loc in selectedLocations:
		cost = random.randrange(100, 1001)
		costPerSlot = random.randrange(10, 201)
		capacity = random.randrange(1, maxCapacity + 1)
		stations.append((loc, cost, costPerSlot, capacity))
	stations = sorted(stations, key = lambda station: station[0])
	
	# find all locations within <= 10 of any station
	auxGraph = graph.copy()
	auxGraph.add_node("root")
	for loc in selectedLocations:
		auxGraph.add_edge("root", loc, distance = 0)
	distanceDict = networkx.single_source_dijkstra_path_length(auxGraph, "root", weight = "distance", cutoff = 10)
	closeLocations = distanceDict.keys()
	# remove artificial root vertex from our list of potential origins/destinations
	del distanceDict['root']
	
	# verify
	# close = list()
	# far = list()
	# for loc in graph.nodes():
		# if(loc in closeLocations):
			# tempDistances = networkx.single_source_dijkstra_path_length(graph, loc, weight = "distance")
			# stationDistances = list()
			# for station in selectedLocations:
				# stationDistances.append(tempDistances[station])
			# close.append(min(stationDistances))
		# else:
			# tempDistances = networkx.single_source_dijkstra_path_length(graph, loc, weight = "distance")
			# stationDistances = list()
			# for station in selectedLocations:
				# stationDistances.append(tempDistances[station])
			# far.append(min(stationDistances))
	# print("max close: " + str(max(close)))
	# print("min far: " + str(min(far)))

	trips = list()
	for i in range(0, tripCount):
		origin = random.choice(list(closeLocations))
		destination = random.choice(list(closeLocations))
		begintime = random.randrange(0, maxTime)
		endtime = random.randrange(begintime + 1, maxTime + 1)
		batteryconsumption = random.randrange(minBattery, maxBattery + 1)
		if(uniformProfit):
			profit = 1
		else:
			profit = alpha * (endtime - begintime) * 100 + gamma * batteryconsumption
		trips.append((origin, destination, begintime, endtime, batteryconsumption, profit))

	# log seed value
	seedfile = open("seeds.log", "a")
	seedfile.write(filename + ": " + str(randomstate) + "\n")
	seedfile.close()
	
	# write instance file
	instancefile = open(filename, "w")
	instancefile.write("# vertexCount edgeCount stationCount tripCount\n")
	instancefile.write("%i %i %i %i\n" % (vertexCount, edgeCount, stationCount, tripCount))
	
	instancefile.write("# edges\n")
	instancefile.write("# source target distance\n")
	for edge in graph.edges(data = True):
		instancefile.write(str(edge[0]) + " " + str(edge[1]) + " " + str(edge[2]['distance']) + "\n")
		#for part in edge:
		#	instancefile.write(str(part) + " ")
		#instancefile.write("\n")
	
	instancefile.write("# stations\n")
	instancefile.write("# location cost costPerSlot capacity\n")
	for station in stations:
		for part in station:
			instancefile.write(str(part) + " ")
		instancefile.write("\n")
	
	instancefile.write("# trips\n")
	instancefile.write("# origin destination begintime endtime batteryconsumption profit\n")
	for trip in trips:
		for part in trip:
			instancefile.write(str(part) + " ")
		instancefile.write("\n")
	
	instancefile.close()


# iterate through possible values
for dimensions in [(30, 30)]:
	for stationCount in [10, 25, 50]:
		for maxCapacity in [20]:
			for tripCount in [10, 25, 50, 75, 100]:
				for batteryLimits in [(1, 50), (1, 100)]:
					for maxTime in [15, 30]:
						for index in [1, 2, 3]:
							writeInstance(dimensions[0], dimensions[1], stationCount, maxCapacity, tripCount, batteryLimits[0], batteryLimits[1], False, maxTime, index)
	





