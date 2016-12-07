import socket #import socket module
serverSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM) # Define a sever socket
server_port = 5555 # This is the port number of the webserver
server_ip= '192.168.1.106' #ip address of the web server
serverSocket.bind(('', server_port)) # To Bind the server
serverSocket.listen(1) # Server can accept only 1 http connection at an instant time
print 'Server IP Address is',server_ip # Printing the Server IP address
print 'Server Port is',server_port # Printing the Server Port Number
print 'Server is prepared'
while True:
    print 'Your Server is ready to display HTML Pages...'
    connectionSocket, address = serverSocket.accept() # Accepting Client Connection
    print 'Client IP address and Port are',address # Printing the address and port of Client
    try:
            message = connectionSocket.recv(1024) # receive data from socket
            print message # Printing the data received from the socket
            filename = message.split()[1] # splitting message
            file1 = open(filename[1:],"rb") # To Open File using Read Binary mode
            outputdata = file1.read()
            connectionSocket.send('HTTP/1.1 200 OK\r\n\r\n') #Send one HTTP header line into socket
            for i in range(0, len(outputdata)):
                 connectionSocket.send(outputdata[i]) #Send the content of the requested file to the client
            print "Success"
    except IOError:
            connectionSocket.send('404 Not Found') #Error message for files not present in the server 
            connectionSocket.close() #Close client socket 
            print 'Server socket closed... '
            serverSocket.close() #Close Server socket 
