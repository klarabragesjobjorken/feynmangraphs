import networkx as nx
import matplotlib.pyplot as plt

# COUNTS DOUBLE EDGES
def double_edges(graph):
    doubles = 0
    for edge in graph.edges:
        if graph.number_of_edges(edge[0], edge[1]) == 2:
            doubles += 1
    return doubles // 2

# COUNTS TRIPLE EDGES
def triple_edges(graph):
    triples = 0
    for edge in graph.edges:
        if graph.number_of_edges(edge[0], edge[1]) == 3:
            triples += 1
    return triples // 3

# CHECKS FOR POTENTIAL ISOMORPHISM
def maybe_isomorphic(graph1, graph2):
    if double_edges(graph1) == double_edges(graph2):
        if triple_edges(graph1) == triple_edges(graph2):
            if len(nx.triangles(graph1)) == len(nx.triangles(graph2)):
                return True
    else:
        return False

# ASKS FOR WHAT GRAPH TYPE TO GENERATE
print("You may generate Feynman graphs or vacuum graphs. Answer with 'Yes' or 'No'.")
feynmanInput = input("Do you want to generate Feynman graphs? ")
if (not feynmanInput == "Yes" or feynmanInput == "yes"):
    vacuumInput = input("Do you want to generate vacuum graphs? ")
    if (not vacuumInput == "Yes" or vacuumInput == "yes"):
        print("Try again")
        exit()

# ASKS FOR AMOUNT OF VERTICES
vertices = int(input("How many vertices do you want? Answer with a positive integer. "))
if feynmanInput == "Yes" or feynmanInput == "yes":
    vertices += 1

# GENERATES VACUUM WITH 2 VERTICES
G_2 = nx.MultiGraph()
for _ in range(4):
    G_2.add_edge(1,2)

# CREATES A LIST TO FILL WITH ALL VACUUM GRAPHS
vacuums = []
for _ in range(vertices):
    vacuums.append([])
vacuums[1].append(G_2)

# CREATES VACUUM GRAPHS
for i in range(1, vertices):

    # OPERATION 1: REMOVE TWO EDGES AND ADD FOUR NEW
    for graph in vacuums[i-1]:
        for edge_1 in graph.edges:
            for edge_2 in graph.edges:
                temporary = graph.copy()
                if not edge_1 == edge_2:
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

                    # CHECKS FOR ISOMORPHISM BETWEEN VACUUM GRAPHS WITH THE SAME NUMBER OF VERTICES
                    # ADDS GRAPHS TO THE LIST OF VACUUM GRAPHS
                    if len(vacuums[i]) != 0:
                        check = 0
                        isIsomorphic = False
                        while check < len(vacuums[i]):
                            if maybe_isomorphic(temporary, vacuums[i][check]) == True:
                                if nx.is_isomorphic(temporary, vacuums[i][check]) == True:
                                    isIsomorphic = True
                                    check = len(vacuums[i])
                            check += 1
                        if isIsomorphic == False:
                            vacuums[i].append(temporary)
                    else:
                        vacuums[i].append(temporary)

    # OPERATION 2: REMOVE ONE EDGE AND ADD FIVE NEW (TWO SINGLE EDGES AND ONE TRIPLE EDGE)
    for graph in vacuums[i-2]:
        for edge in graph.edges:
            temporary2 = graph.copy()
            start = edge[0]
            end = edge[1]
            temporary2.remove_edge(start, end)
            temporary2.add_edge(start, (i+1))
            temporary2.add_edge(end, (i+2))
            for _ in range(3):
                temporary2.add_edge((i+1), (i+2))
            
            # CHECKS FOR ISOMORPHISM BETWEEN VACUUM GRAPHS WITH THE SAME NUMBER OF VERTICES
            # ADDS GRAPHS TO THE LIST OF VACUUM GRAPHS
            if len(vacuums[i]) != 0:
                check = 0
                isIsomorphic = False
                while check < len(vacuums[i]):
                    if maybe_isomorphic(temporary2, vacuums[i][check]) == True:
                        if nx.is_isomorphic(temporary2, vacuums[i][check]) == True:
                            isIsomorphic = True
                            check = len(vacuums[i])
                    check += 1
                if isIsomorphic == False:
                    vacuums[i].append(temporary2)
            else:
                vacuums[i].append(temporary2)

# CREATES FEYNMAN GRAPHS IF REQUESTED
if (feynmanInput == "Yes" or feynmanInput == "yes"):

    # CREATES A LIST TO FILL WITH FEYNMAN GRAPHS
    feynmans = []
    for _ in range(vertices):
        feynmans.append([])

    # CREATES THE FEYNMAN GRAPHS
    for i in range(1, vertices):
        for vacuum in vacuums[i]:
            sameFeynmans = []

            # ENSURES THAT THERE ARE NO CUT VERTICES AND GENERATES FEYNMAN GRAPHS
            if len(list(nx.articulation_points(vacuum))) == 0:
                for node in vacuum.nodes:
                    copy = vacuum.copy()
                    neighbors = []
                    for edge in copy.edges(node):
                        if edge[0] == node:
                            neighbors.append(edge[1])
                        else:
                            neighbors.append(edge[0])
                    copy.remove_node(node)

                    # ENSURES CONNECTEDNESS AND THAT THERE ARE NO CUT EDGES
                    if nx.is_connected(copy) and nx.has_bridges(copy):
                        copy.add_edge('a', neighbors[0])
                        copy.add_edge('b', neighbors[1])
                        copy.add_edge('c', neighbors[2])
                        copy.add_edge('d', neighbors[3])

                        # CHECKS FOR ISOMORPHISM BETWEEN FEYNMAN GRAPHS CREATED FROM THE SAME VACUUMS
                        # ADDS GRAPHS TO A TEMPORARY LIST
                        if len(sameFeynmans) == 0:
                            sameFeynmans.append(copy)
                        else:
                            amount = 0
                            isIsomorphic = False
                            while amount < len(sameFeynmans):
                                if maybe_isomorphic(sameFeynmans[amount], copy) == True:
                                    if nx.is_isomorphic(sameFeynmans[amount], copy) == True:
                                        isIsomorphic = True
                                        amount = len(sameFeynmans)
                                amount += 1
                            if isIsomorphic == False:
                                sameFeynmans.append(copy)

            # ADDS GRAPHS FROM THE TEMPORARY LIST TO THE LIST OF FEYNMAN GRAPHS
            for j in range(len(sameFeynmans)):
                feynmans[i].append(sameFeynmans[j])

    # PLOTS ALL FEYNMAN GRAPHS WITH THE INPUTED AMOUNT OF VERTICES
    for i in range(len(feynmans[vertices-1])):
        G_show = feynmans[vertices-1][i]

        # SELECTS A LAYOUT DEPENDING ON THE AMOUNT OF VERTICES
        if vertices <= 6:
            pos = nx.planar_layout(G_show)

        else:
            pos = nx.random_layout(G_show)

        # CREATES A LIST OF NODES AND REMOVES THE EXTERNAL VERTICES
        node_list = list(G_show.nodes())
        node_list.remove('a')
        node_list.remove('b')
        node_list.remove('c')
        node_list.remove('d')

        # DRAWS THE GRAPHS
        nx.draw_networkx_nodes(G_show, pos, nodelist=node_list, node_size=100, alpha=1)
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

        # SELECTS A LAYOUT DEPENDING ON THE AMOUNT OF VERTICES
        if vertices <= 6:
            pos = nx.planar_layout(G_show)
        else:
            pos = nx.random_layout(G_show)
        
        # CREATES A LIST OF NODES AND REMOVES THE EXTERNAL VERTICES
        node_list = list(G_show.nodes())
        node_list.remove('a')
        node_list.remove('b')
        node_list.remove('c')
        node_list.remove('d')

        # DRAWS THE GRAPHS
        nx.draw_networkx_nodes(G_show, pos, nodelist=node_list, node_size=100, alpha=1)
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