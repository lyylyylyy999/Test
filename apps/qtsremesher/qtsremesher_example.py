import matplotlib.pyplot as plt
#from mpl_toolkits.mplot3d import Axes3D
from fealpy.backend import bm
from fealpy.mesh import TriangleMesh
from fealpy.mesher import SphereSurfaceMesher
from fealpy.mmesh.tool import high_order_meshploter

from qtsremesher import QTSRemesher 

bm.set_backend('numpy')

mesher = SphereSurfaceMesher()
mesh = mesher.init_mesh(2)
mesh.uniform_refine(3)

node = mesh.entity('node')
cell = mesh.entity('cell')

remesher = QTSRemesher(node, cell)
help(remesher)
