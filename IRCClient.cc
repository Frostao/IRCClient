#include "IRCClient.h"

//#include <stdio.h>
//#include <gtk/gtk.h>
//#include "talk-client.c"



int open_client_socket(char * host, int port) {
    // Initialize socket address structure
    struct  sockaddr_in socketAddress;
    
    // Clear sockaddr structure
    memset((char *)&socketAddress,0,sizeof(socketAddress));
    
    // Set family to Internet
    socketAddress.sin_family = AF_INET;
    
    // Set port
    socketAddress.sin_port = htons((u_short)port);
    
    // Get host table entry for this host
    struct  hostent  *ptrh = gethostbyname(host);
    if ( ptrh == NULL ) {
        perror("gethostbyname");
        exit(1);
    }
    
    // Copy the host ip address to socket address structure
    memcpy(&socketAddress.sin_addr, ptrh->h_addr, ptrh->h_length);
    
    // Get TCP transport protocol entry
    struct  protoent *ptrp = getprotobyname("tcp");
    if ( ptrp == NULL ) {
        perror("getprotobyname");
        exit(1);
    }
    
    // Create a tcp socket
    int sock = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }
    
    // Connect the socket to the specified server
    if (connect(sock, (struct sockaddr *)&socketAddress,
                sizeof(socketAddress)) < 0) {
        perror("connect");
        exit(1);
    }
    
    return sock;
}


int sendCommand(char *  host, int port, char * command, char * response) {
    
    int sock = open_client_socket( host, port);
    
    if (sock<0) {
        return 0;
    }
    
    // Send command
    write(sock, command, strlen(command));
    write(sock, "\r\n",2);
    
    //Print copy to stdout
    write(1, command, strlen(command));
    write(1, "\r\n",2);
    
    // Keep reading until connection is closed or MAX_REPONSE
    int n = 0;
    int len = 0;
    while ((n=read(sock, response+len, MAX_RESPONSE - len))>0) {
        len += n;
    }
    response[len]=0;
    
    printf("response:\n%s\n", response);
    
    close(sock);
    
    return 1;
}


void update_list_rooms() {
    GtkTreeIter iter;
    int i;
    //gtk_list_store_clear(GTK_LIST_STORE (list_rooms));
    /* Add some messages to the window */
    //printf("%d",numberOfRooms);
    gint n_rows;
    if (list_rooms != NULL) {
        n_rows = gtk_tree_model_iter_n_children(GTK_TREE_MODEL (list_rooms),NULL);
    } else{
        n_rows = 0;
    }
    if (n_rows < numberOfRooms) {
        for (i = n_rows; i < numberOfRooms; i++) {
            
            gchar *msg = g_strdup_printf ("%s", chatRooms[i].name);
            gtk_list_store_append (GTK_LIST_STORE(list_rooms), &iter);
            
            gtk_list_store_set (GTK_LIST_STORE (list_rooms), &iter, 0, msg, -1);
            
            g_free (msg);
            
        }
    }
    
}
bool switchedRoom;
void update_list_users() {
    if (switchedRoom) {
        gtk_list_store_clear(list_users);
        GtkTreeIter iter;
        int i;
//        gint n_rows;
//        if (list_users != NULL) {
//            n_rows = gtk_tree_model_iter_n_children(GTK_TREE_MODEL (list_users),NULL);
//        } else{
//            n_rows = 0;
//        }
        /* Add some messages to the window */
        
        if (currentRoom != NULL) {
            //if (n_rows < currentRoom->amountOfUsers) {
                for (i = 0; i < currentRoom->amountOfUsers; i++) {
                    gchar *msg = g_strdup_printf ("%s", currentRoom->users[i]);
                    gtk_list_store_append (GTK_LIST_STORE (list_users), &iter);
                    gtk_list_store_set (GTK_LIST_STORE (list_users), &iter, 0, msg, -1);
                    g_free (msg);
                }
                
            //}
        }
        switchedRoom = false;
    }
    
}


void enterRoom(char* roomname){
    char * response = new char[MAX_RESPONSE];
    char * command = (char*)malloc(100*sizeof(char));
    sprintf(command,"ENTER-ROOM %s %s %s", usrname, pswd, roomname);
    sendCommand(serverAddress, port, command, response);
    sprintf(command,"SEND-MESSAGE %s %s %s %s entered room", usrname, pswd, roomname,usrname);
    sendCommand(serverAddress, port, command, response);
    for (int i = 0; i<numberOfRooms; i++) {
        if (!strcmp(chatRooms[i].name, roomname)) {
            currentRoom = &chatRooms[i];
            break;
        }
    }
    switchedRoom = true;
}

void leaveRoom(char * roomname) {
    char * response = new char[MAX_RESPONSE];
    char * command = (char*)malloc(100*sizeof(char));
    sprintf(command,"SEND-MESSAGE %s %s %s %s left room", usrname, pswd, roomname,usrname);
    sendCommand(serverAddress, port, command, response);
    sprintf(command,"LEAVE-ROOM %s %s %s", usrname, pswd, roomname);
    sendCommand(serverAddress, port, command, response);
    currentRoom = NULL;
}

char * enteredRoom;

gboolean view_selection_func (GtkTreeSelection *selection,
                     GtkTreeModel     *model,
                     GtkTreePath      *path,
                     gboolean          path_currently_selected,
                     gpointer          userdata)
{
    GtkTreeIter iter;
    
    if (gtk_tree_model_get_iter(model, &iter, path))
    {
        gchar *name;
        
        gtk_tree_model_get(model, &iter, 0, &name, -1);
        
        if (!path_currently_selected)
        {
            g_print ("%s is going to be selected.\n", name);
//            if (currentRoom != NULL) {
//                leaveRoom(currentRoom->name);
//            }
            if (enteredRoom != NULL) {
                if (!strcmp(enteredRoom, name)) {
                    
                } else {
                    enterRoom(name);
                    enteredRoom = strdup(name);
                }
            } else {
                enterRoom(name);
                enteredRoom = strdup(name);
            }
            
            
        }
        else
        {
            g_print ("%s is going to be unselected.\n", name);
        }
        
        g_free(name);
    }
    return TRUE; /* allow selection state to change */
}


/* Create the list of "messages" */
static GtkWidget *create_list( const char * titleColumn, GtkListStore *model, bool selectionFunc)
{
    GtkWidget *scrolled_window;
    GtkWidget *tree_view;
    //GtkListStore *model;
    GtkCellRenderer *cell;
    GtkTreeViewColumn *column;
    

    
    /* Create a new scrolled window, with scrollbars only if needed */
    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    
    //model = gtk_list_store_new (1, G_TYPE_STRING);
    tree_view = gtk_tree_view_new ();

    gtk_container_add (GTK_CONTAINER (scrolled_window), tree_view);
    gtk_tree_view_set_model (GTK_TREE_VIEW (tree_view), GTK_TREE_MODEL (model));

    gtk_widget_show (tree_view);
    
    cell = gtk_cell_renderer_text_new ();
    
    column = gtk_tree_view_column_new_with_attributes (titleColumn,
                                                       cell,
                                                       "text", 0,
                                                       NULL);
    
    gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view),
                                 GTK_TREE_VIEW_COLUMN (column));
    if (selectionFunc) {
        GtkTreeSelection * selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
        gtk_tree_selection_set_select_function(selection, view_selection_func, NULL,NULL);
    }
    
    return scrolled_window;
}


/* Add some text to our text widget - this is a callback that is invoked
 when our window is realized. We could also force our window to be
 realized with gtk_widget_realize, but it would have to be part of
 a hierarchy first */

static void insert_text( GtkTextBuffer *buffer, const char * initialText )
{
    GtkTextIter iter;
    GtkTextIter iter2;
    
    
    gtk_text_buffer_get_start_iter (buffer, &iter);
    gtk_text_buffer_get_end_iter (buffer, &iter2);
    gtk_text_buffer_delete(buffer, &iter, &iter2);
    gtk_text_buffer_insert (buffer, &iter, initialText,-1);
}

static void sendButtonClicked(GtkWidget *widget, gpointer data) {
    //    GtkWidget *newWindow;
    //    newWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    //    gtk_container_set_border_width (GTK_CONTAINER (newWindow), 10);
    //    gtk_widget_set_size_request (GTK_WIDGET (newWindow), 450, 400);
    //    gtk_widget_show(newWindow);
    
    char * response = new char[MAX_RESPONSE];
    char * command = (char*)malloc(MAX_RESPONSE*sizeof(char));
    
    GtkTextIter startiter;
    GtkTextIter enditer;
    gtk_text_buffer_get_start_iter(messageBuffer, &startiter);
    gtk_text_buffer_get_end_iter(messageBuffer, &enditer);
    char * message = gtk_text_buffer_get_text(messageBuffer, &startiter, &enditer, false);
    
    sprintf(command,"SEND-MESSAGE %s %s %s %s", usrname, pswd, currentRoom->name,message);
    sendCommand(serverAddress, port, command, response);
    
    
}


/* Create a scrolled text area that displays a "message" */
static GtkWidget *create_text(bool editable)
{
    GtkWidget *scrolled_window;
    GtkWidget *view;
    
    view = gtk_text_view_new ();
    if (editable) {
        messageBuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
    } else {
        messagesBuffer = gtk_text_view_get_buffer (GTK_TEXT_VIEW (view));
    }
    
    (GTK_TEXT_VIEW (view))->editable = editable;
    scrolled_window = gtk_scrolled_window_new (NULL, NULL);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_window),
                                    GTK_POLICY_AUTOMATIC,
                                    GTK_POLICY_AUTOMATIC);
    
    gtk_container_add (GTK_CONTAINER (scrolled_window), view);
    //insert_text (buffer, initialText);
    gtk_widget_show_all (scrolled_window);
    
    return scrolled_window;
}

static void createRoomClicked(GtkWidget *widget, gpointer data) {
    GtkWidget *newWindow;
    newWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_container_set_border_width (GTK_CONTAINER (newWindow), 10);
    gtk_widget_set_size_request (GTK_WIDGET (newWindow), 300, 100);
    gtk_window_set_title (GTK_WINDOW (newWindow), "IRCServer Client Sign Up");
    
    GtkWidget *table = gtk_table_new (2, 6, TRUE);
    gtk_container_add (GTK_CONTAINER (newWindow), table);
    gtk_table_set_row_spacings(GTK_TABLE (table), 5);
    gtk_table_set_col_spacings(GTK_TABLE (table), 5);
    gtk_widget_show (table);
    
    GtkWidget *roomnameLabel = gtk_label_new("Room name:");
    gtk_table_attach_defaults (GTK_TABLE (table), roomnameLabel, 0, 2, 0, 1);
    gtk_widget_show(roomnameLabel);
    
    GtkWidget * roomname = gtk_entry_new ();
    gtk_entry_set_max_length (GTK_ENTRY (roomname), 50);
    gtk_table_attach_defaults (GTK_TABLE (table), roomname, 2, 6, 0, 1);
    gtk_widget_show (roomname);
    
    GtkWidget *signup_button = gtk_button_new_with_label ("Create");
    Arguments * twoFields = (Arguments *)malloc(sizeof(Arguments));
    twoFields->arg1 = roomname;
    twoFields->arg2 = newWindow;
    g_signal_connect(signup_button, "clicked", G_CALLBACK(trytoCreateRoom), twoFields);
    gtk_table_attach_defaults(GTK_TABLE (table), signup_button, 0, 6, 1, 2);
    gtk_widget_show (signup_button);
    
    gtk_widget_show(newWindow);
}

static void trytoCreateRoom(GtkWidget *widget, gpointer data) {
    char * response = new char[MAX_RESPONSE];
    Arguments * args = (Arguments *)data;
    
    char * roomname =(char*) gtk_entry_get_text(GTK_ENTRY(args->arg1));
    
    char * command = (char*)malloc(100*sizeof(char));
    sprintf(command,"CREATE-ROOM %s %s %s", usrname,pswd,roomname);
    sendCommand(serverAddress, port, command, response);
    
    GtkWidget *dialog;
    dialog = gtk_message_dialog_new(NULL,
                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                    GTK_MESSAGE_INFO,
                                    GTK_BUTTONS_OK,
                                    response, "title");
    gtk_window_set_title(GTK_WINDOW(dialog), "Information");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    gtk_widget_destroy(GTK_WIDGET(args->arg2));
}


static void trytoSignup(GtkWidget *widget, gpointer data) {
    char * response = new char[MAX_RESPONSE];
    Arguments * args = (Arguments *)data;
    GtkWidget * user = args->arg1;
    GtkWidget * pass = args->arg2;
    char * username =(char*) gtk_entry_get_text(GTK_ENTRY(user));
    char * password =(char*) gtk_entry_get_text(GTK_ENTRY(pass));
    
    char * command = (char*)malloc(100*sizeof(char));
    sprintf(command,"ADD-USER %s %s", username,password);
    sendCommand(serverAddress, port, command, response);
    
    GtkWidget *dialog;
    dialog = gtk_message_dialog_new(NULL,
                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                    GTK_MESSAGE_INFO,
                                    GTK_BUTTONS_OK,
                                    response, "title");
    gtk_window_set_title(GTK_WINDOW(dialog), "Information");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    gtk_widget_destroy(GTK_WIDGET(args->arg3));
}



void showSignupWindow(){
    GtkWidget *newWindow;
    newWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_container_set_border_width (GTK_CONTAINER (newWindow), 10);
    gtk_widget_set_size_request (GTK_WIDGET (newWindow), 400, 100);
    gtk_window_set_title (GTK_WINDOW (newWindow), "IRCServer Client Sign Up");
    
    GtkWidget *table = gtk_table_new (3, 6, TRUE);
    gtk_container_add (GTK_CONTAINER (newWindow), table);
    gtk_table_set_row_spacings(GTK_TABLE (table), 5);
    gtk_table_set_col_spacings(GTK_TABLE (table), 5);
    gtk_widget_show (table);

    GtkWidget *usernameLabel = gtk_label_new("Username:");
    gtk_table_attach_defaults (GTK_TABLE (table), usernameLabel, 0, 2, 0, 1);
    gtk_widget_show(usernameLabel);
    
    GtkWidget * username = gtk_entry_new ();
    gtk_entry_set_max_length (GTK_ENTRY (username), 50);
    gtk_table_attach_defaults (GTK_TABLE (table), username, 2, 6, 0, 1);
    gtk_widget_show (username);
    
    GtkWidget *passwordLabel = gtk_label_new("Password:");
    gtk_table_attach_defaults (GTK_TABLE (table), passwordLabel, 0, 2, 1, 2);
    gtk_widget_show(passwordLabel);
    
    GtkWidget * password = gtk_entry_new ();
    gtk_entry_set_visibility (GTK_ENTRY (password),false);
    gtk_table_attach_defaults (GTK_TABLE (table), password, 2, 6, 1, 2);
    gtk_widget_show (password);
    
    GtkWidget *signup_button = gtk_button_new_with_label ("Create");
    Arguments * twoFields = (Arguments *)malloc(sizeof(Arguments));
    twoFields->arg1 = username;
    twoFields->arg2 = password;
    twoFields->arg3 = newWindow;
    g_signal_connect(signup_button, "clicked", G_CALLBACK(trytoSignup), twoFields);
    gtk_table_attach_defaults(GTK_TABLE (table), signup_button, 0, 6, 2, 3);
    gtk_widget_show (signup_button);
    
    gtk_widget_show(newWindow);
}

static void createAccountClicked(GtkWidget *widget, gpointer data) {
    
    if (data != NULL) {
        Arguments * args = (Arguments *)data;
        GtkWidget * user = args->arg1;
        GtkWidget * pass = args->arg2;
        serverAddress =(char*) gtk_entry_get_text(GTK_ENTRY(user));
        port = atoi((char*) gtk_entry_get_text(GTK_ENTRY(pass)));

    }
    
    
    showSignupWindow();
}

static void leaveRoomClicked(GtkWidget *widget, gpointer data) {
    leaveRoom(currentRoom->name);
}


void showMainWindow(){
    GtkWidget *window;
    GtkWidget *list;
    GtkWidget *messages;
    GtkWidget *myMessage;
    
    
    
    window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title (GTK_WINDOW (window), "IRC Client");
    g_signal_connect (window, "destroy",
                      G_CALLBACK (gtk_main_quit), NULL);
    gtk_container_set_border_width (GTK_CONTAINER (window), 10);
    gtk_widget_set_size_request (GTK_WIDGET (window), 600, 500);
    
    // Create a table to place the widgets. Use a 7x4 Grid (7 rows x 4 columns)
    GtkWidget *table = gtk_table_new (7, 4, TRUE);
    gtk_container_add (GTK_CONTAINER (window), table);
    gtk_table_set_row_spacings(GTK_TABLE (table), 5);
    gtk_table_set_col_spacings(GTK_TABLE (table), 5);
    gtk_widget_show (table);
    
    // Add list of rooms. Use columns 0 to 4 (exclusive) and rows 0 to 4 (exclusive)
    list_rooms = gtk_list_store_new (1, G_TYPE_STRING);
    
    update_list_rooms();
    list = create_list ("Rooms", list_rooms,true);
    gtk_table_attach_defaults (GTK_TABLE (table), list, 2, 4, 0, 2);
    gtk_widget_show (list);
    
    // Add list of users. Use columns 0 to 4 (exclusive) and rows 0 to 4 (exclusive)
    list_users = gtk_list_store_new (1, G_TYPE_STRING);
    update_list_users();
    list = create_list ("Users in the room", list_users, false);
    gtk_table_attach_defaults (GTK_TABLE (table), list, 0, 2, 0, 2);
    gtk_widget_show (list);
    
    // Add messages text. Use columns 0 to 4 (exclusive) and rows 4 to 7 (exclusive)
    messages = create_text (false);
    gtk_table_attach_defaults (GTK_TABLE (table), messages, 0, 4, 2, 5);
    gtk_widget_show (messages);
    // Add messages text. Use columns 0 to 4 (exclusive) and rows 4 to 7 (exclusive)
    
    myMessage = create_text (true);
    gtk_table_attach_defaults (GTK_TABLE (table), myMessage, 0, 4, 5, 7);
    gtk_widget_show (myMessage);
    
    // Add send button. Use columns 0 to 1 (exclusive) and rows 4 to 7 (exclusive)
    GtkWidget *send_button = gtk_button_new_with_label ("Send");
    g_signal_connect(send_button, "clicked", G_CALLBACK(sendButtonClicked), NULL);
    gtk_table_attach_defaults(GTK_TABLE (table), send_button, 0, 1, 7, 8);
    gtk_widget_show (send_button);
    
    // Add Create Account button. Use columns 0 to 1 (exclusive) and rows 4 to 7 (exclusive)
    GtkWidget *createAccount_button = gtk_button_new_with_label ("Create Account");
    g_signal_connect(createAccount_button, "clicked", G_CALLBACK(createAccountClicked), NULL);
    gtk_table_attach_defaults(GTK_TABLE (table), createAccount_button, 1, 2, 7, 8);
    gtk_widget_show (createAccount_button);
    
    // Add Create room button. Use columns 0 to 1 (exclusive) and rows 4 to 7 (exclusive)
    GtkWidget *createRoom_button = gtk_button_new_with_label ("Create Room");
    g_signal_connect(createRoom_button, "clicked", G_CALLBACK(createRoomClicked), NULL);
    gtk_table_attach_defaults(GTK_TABLE (table), createRoom_button, 2, 3, 7, 8);
    gtk_widget_show (createRoom_button);
    
    // Add send button. Use columns 0 to 1 (exclusive) and rows 4 to 7 (exclusive)
    GtkWidget *leave_button = gtk_button_new_with_label ("Leave Room");
    g_signal_connect(leave_button, "clicked", G_CALLBACK(leaveRoomClicked), NULL);
    gtk_table_attach_defaults(GTK_TABLE (table), leave_button, 3, 4, 7, 8);
    gtk_widget_show (leave_button);
    
    gtk_widget_show (table);
    gtk_widget_show (window);

}



static void trytoSignin(GtkWidget *widget, gpointer data) {
    char * response = new char[MAX_RESPONSE];
    Arguments * args = (Arguments *)data;
    serverAddress =strdup((char*) gtk_entry_get_text(GTK_ENTRY(args->arg1)));
    port = atoi((char*) gtk_entry_get_text(GTK_ENTRY(args->arg2)));
    GtkWidget * user = args->arg3;
    GtkWidget * pass = args->arg4;
    char * username =(char*) gtk_entry_get_text(GTK_ENTRY(user));
    char * password =(char*) gtk_entry_get_text(GTK_ENTRY(pass));
    
    
    usrname = strdup(username);
    pswd = strdup(password);
    
    char * command = (char*)malloc(100*sizeof(char));
    sprintf(command,"GET-ALL-USERS %s %s", username,password);
    sendCommand(serverAddress, port, command, response);
    
    GtkWidget *dialog;
    dialog = gtk_message_dialog_new(NULL,
                                    GTK_DIALOG_DESTROY_WITH_PARENT,
                                    GTK_MESSAGE_INFO,
                                    GTK_BUTTONS_OK,
                                    response, "title");
    gtk_window_set_title(GTK_WINDOW(dialog), "Information");
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
    
    loggedIn = true;
    
    gtk_widget_destroy(GTK_WIDGET(args->arg5));
    showMainWindow();
}



void showLoginWindow() {
    GtkWidget *newWindow;
    newWindow = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_container_set_border_width (GTK_CONTAINER (newWindow), 10);
    gtk_widget_set_size_request (GTK_WIDGET (newWindow), 400, 200);
    gtk_window_set_title (GTK_WINDOW (newWindow), "IRCServer Client Sign In");
    
    GtkWidget *table = gtk_table_new (5, 6, TRUE);
    gtk_container_add (GTK_CONTAINER (newWindow), table);
    gtk_table_set_row_spacings(GTK_TABLE (table), 5);
    gtk_table_set_col_spacings(GTK_TABLE (table), 5);
    gtk_widget_show (table);
    
    GtkWidget *serverLabel = gtk_label_new("Server Addess:");
    gtk_table_attach_defaults (GTK_TABLE (table), serverLabel, 0, 2, 0, 1);
    gtk_widget_show(serverLabel);
    
    GtkWidget * server = gtk_entry_new ();
    gtk_entry_set_max_length (GTK_ENTRY (server), 50);
    gtk_table_attach_defaults (GTK_TABLE (table), server, 2, 6, 0, 1);
    gtk_widget_show (server);
    
    GtkWidget *portLabel = gtk_label_new("Port number:");
    gtk_table_attach_defaults (GTK_TABLE (table), portLabel, 0, 2, 1, 2);
    gtk_widget_show(portLabel);
    
    GtkWidget * port = gtk_entry_new ();
    gtk_table_attach_defaults (GTK_TABLE (table), port, 2, 6, 1, 2);
    gtk_widget_show (port);

    GtkWidget *usernameLabel = gtk_label_new("Username:");
    gtk_table_attach_defaults (GTK_TABLE (table), usernameLabel, 0, 2, 2, 3);
    gtk_widget_show(usernameLabel);
    
    GtkWidget * username = gtk_entry_new ();
    gtk_entry_set_max_length (GTK_ENTRY (username), 50);
    gtk_table_attach_defaults (GTK_TABLE (table), username, 2, 6, 2, 3);
    gtk_widget_show (username);
    
    GtkWidget *passwordLabel = gtk_label_new("Password:");
    gtk_table_attach_defaults (GTK_TABLE (table), passwordLabel, 0, 2, 3, 4);
    gtk_widget_show(passwordLabel);
    
    GtkWidget * password = gtk_entry_new ();
    gtk_entry_set_visibility (GTK_ENTRY (password),false);
    gtk_table_attach_defaults (GTK_TABLE (table), password, 2, 6, 3, 4);
    gtk_widget_show (password);
    
    GtkWidget *signin_button = gtk_button_new_with_label ("Sign in");
    Arguments * fiveFields = (Arguments *)malloc(sizeof(Arguments));
    fiveFields->arg1 = server;
    fiveFields->arg2 = port;
    fiveFields->arg3 = username;
    fiveFields->arg4 = password;
    fiveFields->arg5 = newWindow;
    g_signal_connect(signin_button, "clicked", G_CALLBACK(trytoSignin), fiveFields);
    gtk_table_attach_defaults(GTK_TABLE (table), signin_button, 0, 3, 4, 5);
    gtk_widget_show (signin_button);
    
    GtkWidget *createAccount_button = gtk_button_new_with_label ("Create Account");
    Arguments * twoFields = (Arguments *)malloc(sizeof(Arguments));
    twoFields->arg1 = server;
    twoFields->arg2 = port;
    g_signal_connect(createAccount_button, "clicked", G_CALLBACK(createAccountClicked), twoFields);
    gtk_table_attach_defaults(GTK_TABLE (table), createAccount_button, 3, 6, 4, 5);
    gtk_widget_show (createAccount_button);
    
    gtk_widget_show(newWindow);
}

void storeRooms(char *response) {
    int length = strlen(response);
    int currentNumberOfRooms = 0;
    for (int i = 0; i < length; i++) {
        
        int j = 0;
        if (response[i] == '=') {
            char * room = (char*)malloc(100*sizeof(char));;
            i++;
            
            while (response[i] != '\n') {
                room[j] = response[i];
                j++;
                i++;
            }
            room[j] = '\0';
            //chatRooms[currentNumberOfRooms].name = strdup(room);
            currentNumberOfRooms++;
            if (currentNumberOfRooms > numberOfRooms) {
                chatRooms[currentNumberOfRooms-1].name = strdup(room);
                chatRooms[currentNumberOfRooms].amountOfUsers = 0;
                //printf("c=%d,n=%d\n",currentNumberOfRooms,numberOfRooms);
                numberOfRooms++;
            }
            free(room);
        }
    }
}

void storeUsers(char *response) {
    int length = strlen(response);
    int currentNumberOfUsers = 0;
    int j = 0;
    char * user =(char*) malloc(100*sizeof(char));
    for (int i = 0; i < length; i++) {
        
        
        if (response[i] != '\r' && response[i] != '\n') {
            user[j] = response[i];
            j++;
        } else if(response[i] == '\n' && (i != length-1)){
            user[j] = '\0';
            
            //printf("%s\n",user);
            currentNumberOfUsers++;
            if (currentNumberOfUsers >currentRoom->amountOfUsers ) {
                currentRoom->users[currentNumberOfUsers-1] = strdup(user);
                currentRoom->amountOfUsers++;
            }
            
            j = 0;
        }
    }
    free(user);
}

char * oldMessage;

void* messageThread(void* argument){
    while (1) {
        if (loggedIn) {
            char * response = new char[MAX_RESPONSE];
            char * command = new char[100];
            //printf("%s,%d\n",serverAddress,port);
            sprintf(command,"LIST-ROOMS %s %s", usrname,pswd);
            sendCommand(serverAddress, port, command, response);
            storeRooms(response);
            update_list_rooms();
            
            if (currentRoom != NULL) {
                
                sprintf(command,"GET-USERS-IN-ROOM %s %s %s", usrname,pswd,currentRoom->name);
                sendCommand(serverAddress, port, command, response);
                storeUsers(response);
                update_list_users();
                sprintf(command,"GET-MESSAGES %s %s 0 %s", usrname,pswd,currentRoom->name);
                sendCommand(serverAddress, port, command, response);
                if (oldMessage != NULL) {
                    if (!strcmp(oldMessage, response)) {
                        
                    } else {
                        //gtk_text_buffer_set_text(messagesBuffer,strdup(response),-1);
                        insert_text(messagesBuffer, strdup(response));
                        oldMessage = strdup(response);
                    }
                } else {
                    //gtk_text_buffer_set_text(messagesBuffer,strdup(response),-1);
                    insert_text(messagesBuffer, strdup(response));
                    oldMessage = strdup(response);
                }
                
                
            }
            //printf("%s\n",response);
            delete response;
            delete command;
        }
        
        
        //printf("five seconds later\n");
        sleep(5);
    }
}

int main( int   argc,
         char *argv[] )
{
    numberOfRooms = 0;
    
    gtk_init (&argc, &argv);
    showLoginWindow();
    
    pthread_t tid;
    pthread_create(&tid, NULL, &messageThread, NULL);
    gtk_main ();
    return 0;
}

