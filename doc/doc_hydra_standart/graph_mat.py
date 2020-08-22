from graphviz import Digraph

'''
f = Digraph('G', filename='fsm.gv')
f.attr(size='4,5')

f.attr('node', shape='circle')

with f.subgraph(name='cluster_0') as s:
  s.attr(rank='same')
  f.node('lambert')
  f.node('emission')
  f.node('mirror')
  f.node('ggx')
  f.attr(label='BRDFs')

with f.subgraph(name='cluster_1') as s:
  s.attr(rank='same')
  f.node('lambert_tex')
  f.node('emission_tex')
  f.node('mirror_tex')
  f.node('ggx_tex')
  f.attr(label='Textures')

f.node('ggx_tex_gloss')

#f.node('mix_ggx_lambert_maxcolor')

f.node('tex_wrap_clamp', shape='doublecircle', label='tex_wrap_clamp')
f.node('tex_matrices',  shape='doublecircle')

f.edge('lambert',  'lambert_tex')
f.edge('emission', 'emission_tex')
f.edge('mirror',   'mirror_tex')
f.edge('ggx',      'ggx_tex')
f.edge('ggx',      'ggx_tex_gloss')

f.edge('lambert_tex',   'tex_wrap_clamp')
f.edge('emission_tex',  'tex_wrap_clamp')
f.edge('mirror_tex',    'tex_wrap_clamp')
f.edge('ggx_tex',       'tex_wrap_clamp')
f.edge('ggx_tex_gloss', 'tex_wrap_clamp')
f.edge('tex_wrap_clamp', 'tex_matrices')


#f.edge('ggx',     'mix_ggx_lambert_maxcol')
#f.edge('lambert', 'mix_ggx_lambert_maxcol')
#f.edge('mirror',  'mix_ggx_lambert_maxcol')

f.view()

'''

from graphviz import Digraph

g = Digraph('G', filename='cluster.gv')

g.attr(nodesep='1', ranksep='1')

# NOTE: the subgraph name needs to begin with 'cluster' (all lowercase)
#       so that Graphviz recognizes it as a special cluster subgraph

with g.subgraph(name='cluster_0') as c:
    c.node('tex_color', shape='doublecircle')
    c.node('tex_gloss')
    c.node('tex_wrap_clamp')
    c.node('tex_matrices')
    c.edge('tex_color',  'tex_wrap_clamp')
    c.edge('tex_gloss', 'tex_wrap_clamp') 
    c.edge('tex_wrap_clamp', 'tex_matrices')
    c.attr(label='Textures')

with g.subgraph(name='cluster_1') as c:
    c.node('lambert')
    c.node('emission')
    c.node('mirror')
    c.node('ggx')
    c.attr(label='BRDFs (core)')

with g.subgraph(name='cluster_3') as c:
    c.attr(style='dotted')
    c.node('orennayar',style='dotted')
    c.node('phong',style='dotted')
    c.node('maxphong',style='dotted')
    c.attr(label='BRDFs (extensions)')


with g.subgraph(name='cluster_2') as c:
    c.node('diff', shape = 'diamond', label = 'diff(select)')
    c.node('refl', shape = 'diamond', label = 'refl(select)')
    c.node('mix0', label = '''mix(diff, refl)''', shape = 'diamond')
    c.node('mix1', label = '''mix(diff, refl): \nmax_color''')
    c.node('mix2', label = '''mix(diff, refl): \navg_color''')
    c.node('mix3', label = '''mix(diff, refl): \nper_component''', style='dotted')
    c.node('mix4', label = '''mix(diff, refl): \nsum_color''', style='dotted')
    c.edge('mix0', 'mix1')
    c.edge('mix0', 'mix2')
    c.edge('mix0', 'mix3', style='dotted')
    c.edge('mix0', 'mix4', style='dotted')
    c.attr(label='Mixtures')

g.edge('lambert', 'diff')
g.edge('orennayar', 'diff', style='dotted')
g.edge('ggx',     'refl')
g.edge('mirror',  'refl')
g.edge('phong',  'refl', style='dotted')
g.edge('maxphong',  'refl', style='dotted')

g.edge('lambert',  'tex_color')
g.edge('orennayar',  'tex_color', style='dotted')
g.edge('phong',  'tex_color', style='dotted')
g.edge('maxphong',  'tex_color', style='dotted')
g.edge('emission', 'tex_color')
g.edge('mirror',   'tex_color')
g.edge('ggx',      'tex_color')
g.edge('ggx',      'tex_gloss')

g.edge('diff','mix0')
g.edge('refl','mix0')

g.node('END', shape='doublecircle', label='MAT_LEVEL_1.0')

g.edge('mix1','END')
g.edge('mix2','END')
g.edge('mix3','END', style='dotted')
g.edge('mix4','END', style='dotted')
g.edge('tex_matrices','END')

g.view()



