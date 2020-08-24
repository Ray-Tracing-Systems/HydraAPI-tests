from graphviz import Digraph

class LightGraph:

  def __init__(self, ligh_shapes, advance_light_distr, req_level_name):
    self.ligh_shapes  = ligh_shapes
    self.light_distr  = advance_light_distr # assume basic light distribution is Lambert, i.e. Diffuse.
    self.req_level    = req_level_name
    self.brdfs        = ["lambert", "mirror", "ggx"]
    self.basic_shapes = ["rect","disk","sphere"]
    self.allow_cust_dist = ["rect","disk","sphere","point"]

    self.edges = [(brdf,shape) for brdf  in self.brdfs 
                               for shape in self.ligh_shapes]

    self.vertices = self.brdfs + self.ligh_shapes + self.light_distr

    self.g = Digraph('G', filename=req_level_name)
    self.g.attr(nodesep='1', ranksep='1')
  
  def ShapesWithDistr(self, a_shapes):
    for shape in a_shapes:
      if shape in self.ligh_shapes:
        for distr in self.light_distr:
          self.edges.append((shape,distr))
  
  def PrintTests(self):
    arrow_ends = [x[1] for x in self.edges]
    for v in self.vertices:
      if v in arrow_ends:
        print("test(",v,"\tWITH_FEATURES:\t(", end='')
        for e in self.edges:
          if e[1] == v:
            print(e[0], end=',')
        print(") )")
      
  def Draw(self):
   
    with self.g.subgraph(name='cluster_1') as c:
      for lshape in self.ligh_shapes:
        c.node(lshape, shape='doublecircle')
      c.attr(label='Light Shapes (core)')

    with self.g.subgraph(name='cluster_3') as c:
      for brdf in self.brdfs:
        c.node(brdf)
      c.attr(label='BRDFs (core)')

    with self.g.subgraph(name='cluster_4') as c:
      for distr in self.light_distr:
        num_arrows = 0
        for (a,b) in self.edges:
          if distr == a or distr == b:  
            num_arrows=num_arrows+1
        #print("num_arrows = ", num_arrows)
        if num_arrows <= 1:
          c.node(distr)
        else:    
          c.node(distr, shape='doublecircle')    

      c.attr(label='Light Distribution')
    
    for pair in self.edges:
      self.g.edge(pair[0], pair[1])

    self.g.node('END', shape='Msquare', label=self.req_level)
    self.g.edge('sphere', 'END')
    
    for distr in self.light_distr:
      self.g.edge(distr, 'END')

    for shape in [shape for shape in self.ligh_shapes if shape not in self.allow_cust_dist and shape]:
      self.g.edge(shape, 'END')
  
    self.g.view()


#mygraph = LightGraph(["rect","disk","sphere"], 
#                     ["diffuse"], 
#                      "LIGHT_LEVEL_1.0")

#mygraph = LightGraph(["rect","disk","sphere"], 
#                     ["diffuse","ies","spot"], 
#                      "LIGHT_LEVEL_1.1")

mygraph = LightGraph(["rect","disk","sphere","env","direct","point"], 
                     ["diffuse","spot","ies"], 
                      "LIGHT_LEVEL_1.2")

mygraph.ShapesWithDistr(["rect", "disk", "point"])
mygraph.PrintTests()
mygraph.Draw()



