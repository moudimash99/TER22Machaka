import datetime as dt
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import random
from read_log import read_logs
import threading
import queue


viewlast = 30
# Create figure for plotting
fig = plt.figure()
ax = fig.add_subplot(1, 1, 1)
xs = []
ys = []
zs = []
ws = []
qu = []

red = queue.Queue()
blue = queue.Queue()
green = queue.Queue()
# for i in range(5):
#     # Read temperature (Celsius) from TMP102
#     x = random.randint(100,200)
#     z = random.randint(50,150)
#     w = random.randint(150,250)
#     red.put(w)
#     blue.put(x)
#     green.put(z)


def getcolor(ys,zs,ws):
    if (len(ys) < 12):
        return "Too Early","Black"
    cr,cb,cg =0,0,0
    for i in range(1,11):
        print(i)
        r = ys[-i]
        g = zs[-i]
        b = ws[-i]

        if (r >= g) and (r >= b):
           cr+=1

        elif (g >= r) and (g >= b):
           cg+=1
        else:
           cb+=1
    # Limit x and y lists to 20 items
    if (cr >= cg) and (cr >= cb):
       col = "Red"

    elif (cg >= cr) and (cg >= cb):
       col = "Green"
    else:
       col = "Blue"
    return col,col

# This function is called periodically from FuncAnimation
def animate(i, xs, ys,ws,zs,red,green,blue):
    if(green.qsize() == 0 ):
        return(-1)


    # Add x and y to lists
    xs.append(dt.datetime.now().strftime('%M:%S.%f'))
    r = red.get()
    ys.append(r)
    g = green.get()
    zs.append(g)
    b = blue.get()
    ws.append(b)
    

    col,col2 =  getcolor(ys,zs,ws)
    xs = xs[-viewlast:]
    ys = ys[-viewlast:]
    zs = zs[-viewlast:]
    ws = ws[-viewlast:]

    # Draw x and y lists
    ax.clear()
    ax.plot(xs, ys,color='r', label='red')
    ax.plot(xs, zs,color='g', label='green')
    ax.plot(xs, ws,color='b', label='blue')
    ax.text(0.5, 0.92, col, horizontalalignment='center',verticalalignment='center', transform=ax.transAxes,color=col2)

    # Format plot
    plt.xticks(rotation=45, ha='right')
    plt.subplots_adjust(bottom=0.30)
    plt.title('Sensor readings over Time')
    plt.ylabel('Sensor readings')
x = threading.Thread(target=read_logs, args=(red,green,blue))
# import pdb; pdb.set_trace()
x.start()
# Set up plot to call animate() function periodically
ani = animation.FuncAnimation(fig, animate, fargs=(xs, ys,ws,zs,red,green,blue), interval=10)
plt.show()
