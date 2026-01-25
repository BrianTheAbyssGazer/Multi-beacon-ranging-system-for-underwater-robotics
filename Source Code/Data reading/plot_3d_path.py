from scipy.optimize import least_squares
import numpy as np
from matplotlib import pyplot as plt
import plotly.graph_objects as go

def trilateration_least_squares(beacons, distances):
    def residuals(pos):
        return [np.linalg.norm(pos - beacon) - dist for beacon, dist in zip(beacons, distances)]
    
    # Initial guess: centroid of beacons
    x0 = np.mean(beacons, axis=0)
    
    result = least_squares(residuals, x0)
    return result.x

P1 = np.array([-.2, 0, 0])
P2 = np.array([.25, 0, 0])
P3 = np.array([0.04, 0.04, -.16])
data = np.load('G:\ANU Master Courses\8170 group project\Multi-beacon-ranging-system-for-underwater-robotics\Source Code\Data reading\distances_sink.npz')
distance1, distance2, distance3 = data['arr_1']/10,data['arr_2']/10,data['arr_3']/10
plt.figure(figsize=(12,5))
plt.plot(distance1,label='beacon1')
plt.plot(distance2,label='beacon2')
plt.plot(distance3,label='beacon3')
plt.legend()

L=np.min(np.array([len(distance1),len(distance2),len(distance3)]))
Trilater_drone_path = np.zeros((L,3))
for i in range(L):
    Trilater_drone_path[i,:] = trilateration_least_squares([P1,P2,P3],[distance1[i], distance2[i], distance3[i]])
# Create plot
fig = go.Figure()

fig.add_trace(go.Scatter3d(
    x=Trilater_drone_path[:,0], y=Trilater_drone_path[:,1], z=Trilater_drone_path[:,2],
    mode='lines',
    name='Drone Path',
    line=dict(color='blue')
))
beacons=np.array([P1,P2,P3])
fig.add_trace(go.Scatter3d(
    x=beacons[:,0], y=beacons[:,1], z=beacons[:,2],
    mode='markers',
    name='Beacons',
    marker=dict(size=5, color=['green','red','blue'])
))

fig.update_layout(
    scene=dict(xaxis_title='X(m)', yaxis_title='Y(m)', zaxis_title='Z(m)', aspectmode='data'),
    title='Interactive 3D Drone Trajectory',
    width=800,
    height=600,

)

fig.show()