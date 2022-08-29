from sh import tail
import  re
import threading
import queue
red = queue.Queue()
blue = queue.Queue()
green = queue.Queue()
def animate(i, xs, ys):

    # Read temperature (Celsius) from TMP102

    try:
        temp_c = qu.pop()

    except:
        return


    # Add x and y to lists
    xs.append(dt.datetime.now().strftime('%H:%M:%S.%f'))
    ys.append(temp_c)

    # Limit x and y lists to 20 items
    xs = xs[-2000:]
    ys = ys[-2000:]

    # Draw x and y lists
    ax.clear()
    ax.plot(xs, ys)

    # Format plot
    plt.xticks(rotation=45, ha='right')
    plt.subplots_adjust(bottom=0.30)
    plt.title('Sensoe over Time')
    plt.ylabel('Temperature (deg C)')



def read_logs(red,green,blue):

    if(True):
        for line in tail("-f","-n","-50", "stm32f4.log", _iter=True ):


            s = ""
            l = ((line.replace('\x00',"").strip()))

            i = 0
            for c in l:
                if i % 2 == 1:
                    continue
                else:
                    s += c
            l = s

            # import pdb; pdb.set_trace()
            # print(s)
            v1 = 29
            c = 9


            # import pdb; pdb.set_trace()
            if (s[1] + s[3] + s[5] != "XR1"):
                return

            try:
                x = int(s[v1]+s[v1+2]+s[v1+4])
            except( ValueError):
                try:
                    x = int(s[v1]+s[v1+2])
                except:
                    x = -1
            # qu.append(x)
            print(x)
            color = s[c]
            if (color == "R"):
                red.put(x)
            elif (color == "B"):
                blue.put(x)
            elif (color == "G"):
                green.put(x)
            # plt.show(block = False )
            # print("------------------------------------------------------------------------------------")
            # print(l)


#
# read_logs(red,green,blue)
