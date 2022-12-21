import matplotlib.pyplot as plt

# Lecture des données des fichiers service_time.txt, respond_time.txt et client_in_queue.txt
with open("service_time.txt") as f:
    service_times = [int(x) for x in f.read().split()]

with open("respond_time.txt") as f:
    respond_times = [int(x) for x in f.read().split()]

"""with open("client_in_queue.txt") as f:
    client_in_queue = [int(x) for x in f.read().split()]"""

with open("arrival_time.txt") as f:
    arrival_time = [int(x) for x in f.read().split()]


import statistics

# Calculate and print the mean and variance of the service times data
service_mean = statistics.mean(service_times)
service_variance = statistics.variance(service_times)
print(f"Mean of service times: {service_mean}")
print(f"Variance of service times: {service_variance}")

# Calculate and print the mean and variance of the arrival times data
arrival_mean = statistics.mean(arrival_time)
arrival_variance = statistics.variance(arrival_time)
print(f"Mean of arrival times: {arrival_mean}")
print(f"Variance of arrival times: {arrival_variance}")


# Create a figure for the service times data
fig0 = plt.figure()

# Plot the service times data
plt.hist(arrival_time)
plt.xlabel('Arrival times')
plt.ylabel('Number of clients')

# Display the figure
plt.show()

# Create a figure for the service times data
figO = plt.figure()

# Plot the service times data
plt.hist(service_times)
plt.xlabel('Service times')
plt.ylabel('Number of clients')

# Display the figure
plt.show()

# Create a figure for the respond times data
fig2 = plt.figure()

# Plot the respond times data
plt.hist(respond_times)
plt.xlabel('Respond times')
plt.ylabel('Number of clients')

# Display the figure
plt.show()

"""# Create a figure for the client in queue data
fig3 = plt.figure()

# Plot the client in queue data
plt.plot(client_in_queue)
plt.xlabel('Time')
plt.ylabel('Number of clients in queue')

# Display the figure
plt.show()"""


