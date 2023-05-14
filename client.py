import subprocess
import sys
import subprocess
import PySimpleGUI as sg
from enum import Enum
import argparse
import socket
import threading

keep_connection = False
socket_connected = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

def client_connection(socket_connected, window):
    global keep_connection
    # connection for only one client
    socket_connected.listen(10)
    # Release the lock to allow another client to connect
    while keep_connection:
        conn, addr = socket_connected.accept()
        print("connected by ", addr)
        try:
            operation = readMessage(conn)
            if operation == "SEND_MESSAGE":
                print("SEND MESSAGE condition")
                id = int.from_bytes(conn.recv(1), byteorder='little')
                sender = readMessage(conn)
                message = readMessage(conn)
                window['_SERVER_'].print("s> MESSAGE", id, "FROM", sender, "\n", message, "\nEND")
            elif operation == "SEND_MESS_ACK":
                id = int.from_bytes(conn.recv(1), byteorder='little')
                sender = readMessage(conn)
                receiver = readMessage(conn)
                window['_SERVER_'].print("s> SEND MESSAGE", id, "FROM", sender, "TO", receiver, "OK")
        finally:
            print("closing connection")
            conn.close()
    socket_connected.close()
    print("closing socket")
    # kill thread
    return

def readNumber(sock):
    a = ''
    while True:
        msg = sock.recv(1)
        if (msg == b'\0'):
            break
        a += msg.decode()
    if (a == ''):
        return 0
    return(int(a,10))

def readMessage(sock):
    a = ''
    while True:
        msg = sock.recv(1)
        if (msg == b'\0'):
            break
        a += msg.decode()
        print("a: ", a)
    return a


class client :

    # ******************** TYPES *********************
    # *
    # * @brief Return codes for the protocol methods
    class RC(Enum) :
        OK = 0
        ERROR = 1
        USER_ERROR = 2

    # ****************** ATTRIBUTES ******************
    _server = None
    _port = -1
    _quit = 0
    _username = None
    _alias = None
    _date = None

    # ******************** METHODS *******************
    # *
    # * @param user - User name to register in the system
    # *
    # * @return OK if successful
    # * @return USER_ERROR if the user is already registered
    # * @return ERROR if another error occurred
    
    @staticmethod
    def  register(user, window):
        #  Write your code here
        # create socket
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((client._server, client._port))
        try:
            # operation register
            message = "REGISTER\0" + user + "\0" + client._username + "\0"+ client._date + "\0"
            s.sendall(message.encode("utf-8"))
        finally:
            result = int.from_bytes(s.recv(4), byteorder='little')
        s.close()

        if (result == 0):
            window['_SERVER_'].print("s> REGISTER OK")
        elif (result == 1):
            window['_SERVER_'].print("s> USERNAME IN USE")
        else:
            window['_SERVER_'].print("s> REGISTER FAIL")
        return client.RC.ERROR

    # *
    # 	 * @param user - User name to unregister from the system
    # 	 *
    # 	 * @return OK if successful
    # 	 * @return USER_ERROR if the user does not exist
    # 	 * @return ERROR if another error occurred
    @staticmethod
    def  unregister(user, window):
        #  Write your code here
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((client._server, client._port))
        try:
            message = "UNREGISTER\0" + user + "\0"
            s.sendall(message.encode("utf-8"))
        finally:
            result = int.from_bytes(s.recv(4), byteorder='little')
            s.close()
        if (result == 0):
            window['_SERVER_'].print("s> UNREGISTER OK")
        elif (result == 1):
            window['_SERVER_'].print("s> USER DOES NOT EXIST")
        else:
            window['_SERVER_'].print("s> UNREGISTER FAIL")
            
        client._alias = None
        return client.RC.ERROR


    # *
    # * @param user - User name to connect to the system
    # *
    # * @return OK if successful
    # * @return USER_ERROR if the user does not exist or if it is already connected
    # * @return ERROR if another error occurred
    @staticmethod
    def  connect(user, window):
        #  Write your code here
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        socket_connected = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        # look for a free port
        socket_connected.bind(('0.0.0.0', 0))
        port = socket_connected.getsockname()[1]
        # create a thread and connect there the socket
        global keep_connection, connection_thread
        if keep_connection == False:
            keep_connection = True
            connection_thread = threading.Thread(target=client_connection, name='Daemon', args=(socket_connected, window))
            connection_thread.start()
        s.connect((client._server, client._port))
        try:
            message = "CONNECT\0" + user + "\0" + str(port) + "\0"
            s.sendall(message.encode("utf-8"))
            
        finally:
            # check if there are messages by receiving the first integer
            result = readNumber(s)
            s.close()
        if (result == 0):
            window['_SERVER_'].print("s> CONNECT OK")
        elif result == 1:
            window['_SERVER_'].print("s> CONNECT FAIL, USER DOES NOT EXIST")
        elif result == 2:
            window['_SERVER_'].print("s> USER ALREADY CONNECTED")
        else:
            window['_SERVER_'].print("s> CONNECT FAIL")
        return client.RC.ERROR


    # *
    # * @param user - User name to disconnect from the system
    # *
    # * @return OK if successful
    # * @return USER_ERROR if the user does not exist
    # * @return ERROR if another error occurred
    @staticmethod
    def  disconnect(user, window):
        #  Write your code here
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((client._server, client._port))
        #kill thread and close socket
        global keep_connection
        if keep_connection:
            keep_connection = False
        try:
            s.sendall(b'DISCONNECT\0')
            s.sendall((user).encode("utf-8"))
            s.sendall(b'\0')
         
        finally:
            result = int.from_bytes(s.recv(4), byteorder='little')
            s.close()
            
        if (result == 0):
            window['_SERVER_'].print("s> DISCONNECT", user, "OK")
        elif (result == 1):
            window['_SERVER_'].print("s> DISCONNECT FAIL/USER DOES NOT EXIST")
        elif (result == 2):
            window['_SERVER_'].print("s> DISCONNECT FAIL/USER NOT CONNECTED")
        else:
            window['_SERVER_'].print("s> DISCONNECT FAIL")
            
        return client.RC.ERROR

    # *
    # * @param user    - Receiver user name
    # * @param message - Message to be sent
    # *
    # * @return OK if the server had successfully delivered the message
    # * @return USER_ERROR if the user is not connected (the message is queued for delivery)
    # * @return ERROR the user does not exist or another error occurred
    @staticmethod
    def  send(user, message, window):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((client._server, client._port))
        try:
            # comando
            s.sendall(b'SEND_MESSAGE\0')
            # usuario emisor
            s.sendall((client._alias).encode("utf-8"))
            # port and ip vacio
            s.sendall(b'\0')
            # usuario receptor
            s.sendall(user.encode("utf-8"))
            s.sendall(b'\0')
            # mensaje
            s.sendall((message).encode("utf-8"))
            s.sendall(b'\0')

            #receive message id from socket
            id = int.from_bytes(s.recv(4), byteorder='little')
        finally:
            result = int.from_bytes(s.recv(4), byteorder='little')
            s.close()

        if (result == 0):
            window['_SERVER_'].print("s> SEND OK - MESSAGE", id)
        elif (result == 1):
            window['_SERVER_'].print("s> SEND FAIL/USER DOES NOT EXIST")
        else:
            window['_SERVER_'].print("s> SEND FAIL")
        return client.RC.ERROR

    # *
    # * @param user    - Receiver user name
    # * @param message - Message to be sent
    # * @param file    - file  to be sent

    # *
    # * @return OK if the server had successfully delivered the message
    # * @return USER_ERROR if the user is not connected (the message is queued for delivery)
    # * @return ERROR the user does not exist or another error occurred
    @staticmethod
    def  sendAttach(user, message, file, window):
        window['_SERVER_'].print("s> SENDATTACH MESSAGE OK")
        print("SEND ATTACH " + user + " " + message + " " + file)
        #  Write your code here
        return client.RC.ERROR

    @staticmethod
    def  connectedUsers(window):
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((client._server, client._port))
        try:
            # comando
            s.sendall(b'CONNECTEDUSERS\0')
            # usuario emisor
            print("alias: ", client._alias)
            s.sendall((client._alias).encode("utf-8"))
            s.sendall(b'\0')

        finally:
            result = int.from_bytes(s.recv(4), byteorder='little')
            print("result: ", result)
            if result == 0:
                num_users = int.from_bytes(s.recv(4), byteorder='little')
                i = 1
                string = "s> CONNECTED USERS (" + str(num_users) + " users connected) OK - "
                string += readMessage(s)
                while i < num_users:
                    user = readMessage(s)
                    strint += ",  " + user
                    i += 1
            s.close()

        if (result == 0):
            window['_SERVER_'].print(string)
        elif (result == 1):
            window['_SERVER_'].print("s> CONNECTED USERS FAIL / USER IS NOT CONNECTED")
        else:
            window['_SERVER_'].print("s> CONNECTED USERS FAIL")
        return client.RC.ERROR
        


    @staticmethod
    def window_register():
        layout_register = [[sg.Text('Ful Name:'),sg.Input('Text',key='_REGISTERNAME_', do_not_clear=True, expand_x=True)],
                            [sg.Text('Alias:'),sg.Input('Text',key='_REGISTERALIAS_', do_not_clear=True, expand_x=True)],
                            [sg.Text('Date of birth:'),sg.Input('',key='_REGISTERDATE_', do_not_clear=True, expand_x=True, disabled=True, use_readonly_for_disable=False),
                            sg.CalendarButton("Select Date",close_when_date_chosen=True, target="_REGISTERDATE_", format='%d/%m/%Y',size=(10,1))],
                            [sg.Button('SUBMIT', button_color=('white', 'blue'))]
                            ]

        layout = [[sg.Column(layout_register, element_justification='center', expand_x=True, expand_y=True)]]

        window = sg.Window("REGISTER USER", layout, modal=True)
        choice = None

        while True:
            event, values = window.read()

            if (event in (sg.WINDOW_CLOSED, "-ESCAPE-")):
                break

            if event == "SUBMIT":
                if(values['_REGISTERNAME_'] == 'Text' or values['_REGISTERNAME_'] == '' or values['_REGISTERALIAS_'] == 'Text' or values['_REGISTERALIAS_'] == '' or values['_REGISTERDATE_'] == ''):
                    sg.Popup('Registration error', title='Please fill in the fields to register.', button_type=5, auto_close=True, auto_close_duration=1)
                    continue

                client._username = values['_REGISTERNAME_']
                client._alias = values['_REGISTERALIAS_']
                client._date = values['_REGISTERDATE_']
                break
        window.Close()


    # *
    # * @brief Prints program usage
    @staticmethod
    def usage() :
        print("Usage: python3 py -s <server> -p <port>")


    # *
    # * @brief Parses program execution arguments
    @staticmethod
    def  parseArguments(argv) :
        parser = argparse.ArgumentParser()
        parser.add_argument('-s', type=str, required=True, help='Server IP')
        parser.add_argument('-p', type=int, required=True, help='Server Port')
        args = parser.parse_args()

        if (args.s is None):
            parser.error("Usage: python3 py -s <server> -p <port>")
            return False

        if ((args.p < 1024) or (args.p > 65535)):
            parser.error("Error: Port must be in the range 1024 <= port <= 65535");
            return False;

        client._server = args.s
        client._port = args.p

        return True


    def main(argv):

        if (not client.parseArguments(argv)):
            client.usage()
            exit()

        lay_col = [[sg.Button('REGISTER',expand_x=True, expand_y=True),
                sg.Button('UNREGISTER',expand_x=True, expand_y=True),
                sg.Button('CONNECT',expand_x=True, expand_y=True),
                sg.Button('DISCONNECT',expand_x=True, expand_y=True),
                sg.Button('CONNECTED USERS',expand_x=True, expand_y=True)],
                [sg.Text('Dest:'),sg.Input('User',key='_INDEST_', do_not_clear=True, expand_x=True),
                sg.Text('Message:'),sg.Input('Text',key='_IN_', do_not_clear=True, expand_x=True),
                sg.Button('SEND',expand_x=True, expand_y=False)],
                [sg.Text('Attached File:'), sg.In(key='_FILE_', do_not_clear=True, expand_x=True), sg.FileBrowse(),
                sg.Button('SENDATTACH',expand_x=True, expand_y=False)],
                [sg.Multiline(key='_CLIENT_', disabled=True, autoscroll=True, size=(60,15), expand_x=True, expand_y=True),
                sg.Multiline(key='_SERVER_', disabled=True, autoscroll=True, size=(60,15), expand_x=True, expand_y=True)],
                [sg.Button('QUIT', button_color=('white', 'red'))]
            ]


        layout = [[sg.Column(lay_col, element_justification='center', expand_x=True, expand_y=True)]]

        window = sg.Window('Messenger', layout, resizable=True, finalize=True, size=(1000,400))
        window.bind("<Escape>", "-ESCAPE-")


        while True:
            event, values = window.Read()

            if (event in (None, 'QUIT')) or (event in (sg.WINDOW_CLOSED, "-ESCAPE-")):
                sg.Popup('Closing Client APP', title='Closing', button_type=5, auto_close=True, auto_close_duration=1)
                break

            #if (values['_IN_'] == '') and (event != 'REGISTER' and event != 'CONNECTED USERS'):
             #   window['_CLIENT_'].print("c> No text inserted")
             #   continue

            if (client._alias == None or client._username == None or client._alias == 'Text' or client._username == 'Text' or client._date == None) and (event != 'REGISTER'):
                sg.Popup('NOT REGISTERED', title='ERROR', button_type=5, auto_close=True, auto_close_duration=1)
                continue

            if (event == 'REGISTER'):
                client.window_register()

                if (client._alias == None or client._username == None or client._alias == 'Text' or client._username == 'Text' or client._date == None):
                    sg.Popup('NOT REGISTERED', title='ERROR', button_type=5, auto_close=True, auto_close_duration=1)
                    continue

                window['_CLIENT_'].print('c> REGISTER ' + client._alias)
                client.register(client._alias, window)

            elif (event == 'UNREGISTER'):
                window['_CLIENT_'].print('c> UNREGISTER ' + client._alias)
                client.unregister(client._alias, window)


            elif (event == 'CONNECT'):
                window['_CLIENT_'].print('c> CONNECT ' + client._alias)
                client.connect(client._alias, window)


            elif (event == 'DISCONNECT'):
                window['_CLIENT_'].print('c> DISCONNECT ' + client._alias)
                client.disconnect(client._alias, window)


            elif (event == 'SEND'):
                window['_CLIENT_'].print('c> SEND ' + values['_INDEST_'] + " " + values['_IN_'])

                if (values['_INDEST_'] != '' and values['_IN_'] != '' and values['_INDEST_'] != 'User' and values['_IN_'] != 'Text') :
                    client.send(values['_INDEST_'], values['_IN_'], window)
                else :
                    window['_CLIENT_'].print("Syntax error. Insert <destUser> <message>")


            elif (event == 'SENDATTACH'):

                window['_CLIENT_'].print('c> SENDATTACH ' + values['_INDEST_'] + " " + values['_IN_'] + " " + values['_FILE_'])

                if (values['_INDEST_'] != '' and values['_IN_'] != '' and values['_FILE_'] != '') :
                    client.sendAttach(values['_INDEST_'], values['_IN_'], values['_FILE_'], window)
                else :
                    window['_CLIENT_'].print("Syntax error. Insert <destUser> <message> <attachedFile>")


            elif (event == 'CONNECTED USERS'):
                window['_CLIENT_'].print("c> CONNECTEDUSERS")
                client.connectedUsers(window)



            window.Refresh()

        window.Close()


if __name__ == '__main__':
    client.main([])
    print("+++ FINISHED +++")


