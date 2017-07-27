import MeshSync as ms


ctx = ms.Context()
mesh1 = ctx.addMesh("/pmsMesh")

mesh1.addVertex([0.0, 0.0, 0.0])
mesh1.addVertex([0.0, 0.0, 1.0])
mesh1.addVertex([1.0, 0.0, 1.0])
mesh1.addVertex([1.0, 0.0, 0.0])

mesh1.addUV([0.0, 0.0])
mesh1.addUV([0.0, 1.0])
mesh1.addUV([1.0, 1.0])
mesh1.addUV([1.0, 0.0])

mesh1.addCount(4)
mesh1.addMaterialID(0)
mesh1.addIndex(0)
mesh1.addIndex(1)
mesh1.addIndex(2)
mesh1.addIndex(3)
ctx.send()
