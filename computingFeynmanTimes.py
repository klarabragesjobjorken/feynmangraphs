import networkx as nx
import matplotlib.pyplot as plt
import time

fastCounter = 0
slowCounter = 0

slowIsomorphismSolve = 0
fastIsomorphismSolve = 0

OperationTime = 0
IsomorphismTime = 0
fastIsomorphismTime = 0
slowIsomorphismTime = 0
CutVertexTime = 0
CutEdgeTime = 0
ConnectedTime = 0
Part1Time = 0
Part2Time = 0


def double_edges(graph):
    doubles = 0
    for edge in graph.edges:
        if graph.number_of_edges(edge[0], edge[1]) == 2:
            doubles += 1
    return doubles // 2

def triple_edges(graph):
    triples = 0
    for edge in graph.edges:
        if graph.number_of_edges(edge[0], edge[1]) == 3:
            triples += 1
    return triples // 3

def maybe_isomorphic(graph1, graph2):
    if double_edges(graph1) == double_edges(graph2):
        if triple_edges(graph1) == triple_edges(graph2):
            if len(nx.triangles(graph1)) == len(nx.triangles(graph2)):
                return True
    else:
        return False

print("You may generate Feynman Diagrams or Vacuum diagrams. Answer with 'Yes' or 'No'.")
feynmanInput = input("Do you want to generate Feynman Diagrams? ")
if not (feynmanInput == "Yes" or feynmanInput == "yes"):
    vacuumInput = input("Do you want to generate Vacuum Diagrams? ")
    if not (vacuumInput == "Yes" or vacuumInput == "yes"):
        print("Try again")
        exit()

vertices = int(input("How many vertices do you want? Answer with a positive integer. "))
if (feynmanInput == "Yes" or feynmanInput == "yes"):
    vertices += 1

st = time.time()

G_2 = nx.MultiGraph()
for _ in range(4):
    G_2.add_edge(1,2)

vacuums = []
for _ in range(vertices):
    vacuums.append([])
vacuums[1].append(G_2)

Part1Time1 = time.time()

for i in range(2, vertices):
    for graph in vacuums[i-1]:
        for edge_1 in graph.edges:
            for edge_2 in graph.edges:
                temporary = graph.copy()
                if not edge_1 == edge_2:
                    operationTime1 = time.time()
                    start1 = edge_1[0]
                    end1 = edge_1[1]
                    start2 = edge_2[0]
                    end2 = edge_2[1]
                    temporary.remove_edge(start1, end1)
                    temporary.remove_edge(start2, end2)
                    temporary.add_edge(start1, (i+2))
                    temporary.add_edge(end1, (i+2))
                    temporary.add_edge(start2, (i+2))
                    temporary.add_edge(end2, (i+2))
                    operationTime2 = time.time()
                    OperationTime += (operationTime2 - operationTime1)
                    isomorphismTime1 = time.time()
                    if len(vacuums[i]) != 0:
                        check = 0
                        isIsomorphic = False
                        while check < len(vacuums[i]):
                            fastIsomorphismTime1 = time.time()
                            if maybe_isomorphic(temporary, vacuums[i][check]) == True:
                                slowIsomorphismTime1 = time.time()
                                if nx.is_isomorphic(temporary, vacuums[i][check]) == True:
                                    isIsomorphic = True
                                    check = len(vacuums[i])
                                else: 
                                    slowCounter = 1
                                    slowIsomorphismSolve += 1
                            else: 
                                fastCounter = 1
                                fastIsomorphismSolve += 1
                            fastIsomorphismTime2 = time.time()
                            slowIsomorphismTime2 = time.time()
                            if slowCounter == 1:
                                slowIsomorphismTime += (slowIsomorphismTime2 - slowIsomorphismTime1)
                            if fastCounter == 1:
                                fastIsomorphismTime += (fastIsomorphismTime2 - fastIsomorphismTime1)
                            slowCounter = 0
                            fastCounter = 0
                            check += 1
                        if isIsomorphic == False:
                            vacuums[i].append(temporary)
                    else:
                        vacuums[i].append(temporary)
                    isomorphismTime2 =time.time()
                    IsomorphismTime += (isomorphismTime2 - isomorphismTime1)
    
    for graph in vacuums[i-2]:
        for edge in graph.edges:
            operationTime1 = time.time()
            temporary2 = graph.copy()
            start = edge[0]
            end = edge[1]
            temporary2.remove_edge(start, end)
            temporary2.add_edge(start, i+2)
            temporary2.add_edge(end, (i+1))
            for _ in range(3):
                temporary2.add_edge(i+2, (i+1))
            operationTime2 = time.time()
            OperationTime += (operationTime2 - operationTime1)
            isomorphismTime1 = time.time()
            if len(vacuums[i]) != 0:
                check = 0
                isIsomorphic = False
                while check < len(vacuums[i]):
                    fastIsomorphismTime1 = time.time()
                    if maybe_isomorphic(temporary2, vacuums[i][check]) == True:
                        slowIsomorphismTime1 = time.time()
                        if nx.is_isomorphic(temporary2, vacuums[i][check]) == True:
                            isIsomorphic = True
                            check = len(vacuums[i])
                        else: 
                            slowCounter = 1
                            slowIsomorphismSolve += 1
                    else: 
                        fastCounter = 1
                        fastIsomorphismSolve += 1
                    fastIsomorphismTime2 = time.time()
                    slowIsomorphismTime2 = time.time()
                    if slowCounter == 1:
                        slowIsomorphismTime += (slowIsomorphismTime2 - slowIsomorphismTime1)
                    if fastCounter == 1:
                        fastIsomorphismTime += (fastIsomorphismTime2 - fastIsomorphismTime1)
                    slowCounter = 0
                    fastCounter = 0
                    check += 1
                if isIsomorphic == False:
                    vacuums[i].append(temporary2)
            else:
                vacuums[i].append(temporary2)
            isomorphismTime2 =time.time()
            IsomorphismTime += (isomorphismTime2 - isomorphismTime1)
    print("Vacuums:", len(vacuums[i]))
Part1Time2 = time.time()
Part1Time += (Part1Time2 - Part1Time1)

Part2Time1 = time.time()
if feynmanInput == "Yes" or feynmanInput == "yes":
        feynmans = []
        for _ in range(vertices):
            feynmans.append([])
        for i in range(1, vertices):
            for vacuum in vacuums[i]:
                cutVertexTime1 = time.time()
                cutVerticesAmount = list(nx.articulation_points(vacuum))
                cutVertexTime2 = time.time()
                CutVertexTime += (cutVertexTime2 - cutVertexTime1)
               
                sameFeynmans = []
                if len(cutVerticesAmount) == 0:
                    for node in vacuum.nodes:
                        operationTime1 = time.time()
                        copy = vacuum.copy()
                        neighbors = []
                        for edge in copy.edges(node):
                            if edge[0] == node:
                                neighbors.append(edge[1])
                            else:
                                neighbors.append(edge[2])
                        copy.remove_node(node)
                        operationTime2 = time.time()
                        OperationTime += (operationTime2 - operationTime1)
                        ConnectedTime1 = time.time()
                        ConnectedNess = nx.is_connected(copy)
                        ConnectedTime2 = time.time()
                        ConnectedTime += (ConnectedTime2 - ConnectedTime1)
                        CutEdgeTime1 = time.time()
                        Bridges = nx.has_bridges(copy)
                        CutEdgeTime2 = time.time()
                        CutEdgeTime += (CutEdgeTime2 - CutEdgeTime1)

                        if ConnectedNess == True and Bridges == False:
                            copy.add_edge('a', neighbors[0])
                            copy.add_edge('b', neighbors[1])
                            copy.add_edge('c', neighbors[2])
                            copy.add_edge('d', neighbors[3])
                            isomorphismTime1 = time.time()
                            if len(sameFeynmans) == 0:
                                sameFeynmans.append(copy)
                            else:
                                amount = 0
                                isIsomorphic = False
                                while amount < len(sameFeynmans):
                                    fastIsomorphismTime1 = time.time()
                                    if maybe_isomorphic(sameFeynmans[amount], copy) == True:
                                        slowIsomorphismTime1 = time.time()
                                        if nx.is_isomorphic(sameFeynmans[amount], copy) == True:
                                            isIsomorphic = True
                                            amount = len(sameFeynmans)
                                        else: 
                                            slowCounter = 1
                                            slowIsomorphismSolve += 1
                                    else: 
                                        fastCounter = 1
                                        fastIsomorphismSolve += 1
                                    fastIsomorphismTime2 = time.time()
                                    slowIsomorphismTime2 = time.time()
                                    if slowCounter == 1:
                                        slowIsomorphismTime += (slowIsomorphismTime2 - slowIsomorphismTime1)
                                    if fastCounter == 1:
                                        fastIsomorphismTime += (fastIsomorphismTime2 - fastIsomorphismTime1)
                                    slowCounter = 0
                                    fastCounter = 0
                                    amount += 1
                                if isIsomorphic == False:
                                    sameFeynmans.append(copy)
                            isomorphismTime2 = time.time()
                            IsomorphismTime += (isomorphismTime2 - isomorphismTime1)
                for j in range(len(sameFeynmans)):
                    feynmans[i].append(sameFeynmans[j])
            print("Feynman Diagrams with 4 external edges:", len(feynmans[i]))

        
Part2Time2 = time.time()
Part2Time += (Part2Time2 - Part2Time1)

print("Time for Part 1: ", Part1Time)
print("Time for Part 2:", Part2Time)
print("Time for Applying Operations: ", OperationTime)
print("Time for Checking Isomoprhims Quickly: ", fastIsomorphismTime)
print("Time for Checking Isomorphisms Slowly: ", slowIsomorphismTime)
print("Total Time for Checking Isomorphisms: ", IsomorphismTime)
print("Time for Checking Connectedness ", ConnectedTime)
print("Time for Checking Cut Edges: ", CutEdgeTime)
print("Time for Checking Cut Vertices: ", CutVertexTime)

et = time.time()

print("Total Time: ",et - st)

print("Fast Isomorphism Checks: ", fastIsomorphismSolve)
print("Slow Isomorphism Checks: ", slowIsomorphismSolve)


for i in range(len(feynmans[vertices-1])):
    G_show = feynmans[vertices-1][i]
    pos = nx.planar_layout(G_show)
    nx.draw_networkx_nodes(G_show, pos, node_size=100, alpha=1)
    ax = plt.gca()
    for edge in G_show.edges:
        ax.annotate("",
                    xy=pos[edge[0]], xycoords='data',
                    xytext=pos[edge[1]], textcoords='data',
                    arrowprops=dict(arrowstyle="-", color="0.5",
                                    shrinkA=5, shrinkB=5,
                                    patchA=None, patchB=None,
                                    connectionstyle="arc3,rad=rrr".replace('rrr',str(0.3*edge[2])),
                                    ),
                    )
    plt.axis('off')
    plt.show()
else:
    for i in range(len(vacuums[vertices-1])):
        G_show = vacuums[vertices-1][i]
        pos = nx.planar_layout(G_show)
        nx.draw_networkx_nodes(G_show, pos, node_color='r', node_size=100, alpha=1)
        ax = plt.gca()
        for edge in G_show.edges:
            ax.annotate("",
                        xy=pos[edge[0]], xycoords='data',
                        xytext=pos[edge[1]], textcoords='data',
                        arrowprops=dict(arrowstyle="-", color="0.5",
                                        shrinkA=5, shrinkB=5,
                                        patchA=None, patchB=None,
                                        connectionstyle="arc3,rad=rrr".replace('rrr',str(0.3*edge[2])),
                                        ),
                        )
        plt.axis('off')
        plt.show()