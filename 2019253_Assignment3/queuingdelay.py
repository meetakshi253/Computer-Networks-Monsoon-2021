import matplotlib.pyplot as plt
fp=open("tcp-example3.tr","r")

enqueue = {}
sojourn = {}


for i in fp:
    token = i.split()
    eqdq = token[0]
    time = float(token[1])
    node = token[2]
    node = node[10]
    # if(node!="0"):
    #     continue
    seq = token[36]
    index = node+seq
    if(eqdq=='+'):
        enqueue[index] = time
    elif(eqdq=='-'):
        sojourn[time] = (float(time)-float(enqueue[index]))*1000
plt.xlabel("Dequeue Time (seconds)")
plt.ylabel("Waiting/Sojourn Time (milli seconds)")
plt.title("3 c (b). Queuing Delay with Time (Node 0 and 1)")
plt.grid()
plt.scatter(sojourn.keys(), sojourn.values())
plt.show()