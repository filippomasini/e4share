import sys
import random
import math

def writeInstance(width, height, stationCount, maxCapacity, tripCount, minBattery, maxBattery, uniformProfit, carCount, maxTime, index):
	if(uniformProfit):
		profitStr = "uniform"
	else:
		profitStr = "nonuniform"
	filename = "%ix%i-s%i-cap%i-k%i-b%i-%i-%s-c%i-t%i-%i.loc" % (width, height, stationCount, maxCapacity, tripCount, minBattery, maxBattery, profitStr, carCount, maxTime, index)
	# parameters for calculating a trip's profit
	alpha = 2
	gamma = 1
	# log random "seed"
	randomstate = random.getstate()
	
	#print(filename)
	vertexCount = width * height
	vertices = range(0, vertexCount)
	edgeCount = (width - 1) * height + (height - 1) * width
	if(stationCount > vertexCount):
		stationCount = vertexCount

	edges = list()
	for y in range(0, height):
		for x in range(0, width):
			vertex = y * width + x
			if(x < (width - 1)):
				distance = random.randrange(1, 6)
				edge = (vertex, vertex + 1, distance)
				edges.append(edge)
			if(y < (height - 1)):
				distance = random.randrange(1, 6)
				edge = (vertex, vertex + width, distance)
				edges.append(edge)

	stations = list()
	selectedLocations = random.sample(vertices, stationCount)
	for loc in selectedLocations:
		cost = random.randrange(100, 1001)
		costPerSlot = random.randrange(10, 201)
		capacity = random.randrange(1, maxCapacity + 1)
		stations.append((loc, cost, costPerSlot, capacity))
	stations = sorted(stations, key = lambda station: station[0])

	trips = list()
	for i in range(0, tripCount):
		origin = random.choice(vertices)
		destination = random.choice(vertices)
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
	instancefile.write("# vertexCount edgeCount stationCount carCount tripCount\n")
	instancefile.write("%i %i %i %i %i\n" % (vertexCount, edgeCount, stationCount, carCount, tripCount))
	
	instancefile.write("# edges\n")
	instancefile.write("# source target distance\n")
	for edge in edges:
		for part in edge:
			instancefile.write(str(part) + " ")
		instancefile.write("\n")
	
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
for dimensions in [(10, 10), (10, 20), (20, 20), (20, 30), (30, 30)]:
	for stationCount in [10, 20, 30, 40, 50]:
		for maxCapacity in [10, 20]:
			for tripCount in [10, 20, 30, 40, 50, 75, 100]:
				for batteryLimits in [(1, 50), (40, 100), (1, 100)]:
					for uniformProfit in [True, False]:
						for carCount in [tripCount, round(tripCount / 2), round(tripCount / 5)]:
							for maxTime in [10, 15, 30]:
								for index in [1, 2, 3, 4, 5]:
									writeInstance(dimensions[0], dimensions[1], stationCount, maxCapacity, tripCount, batteryLimits[0], batteryLimits[1], uniformProfit, carCount, maxTime, index)
	





