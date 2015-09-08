#include <stdio.h>
#include <gtk/gtk.h>
#include <time.h>
//#include <curses.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>


GtkListStore * list_rooms;
GtkListStore * list_users;
char * serverAddress;
int port;
char * usrname;
char * pswd;
bool loggedIn;
int numberOfRooms;
GtkTextBuffer *messagesBuffer;
GtkTextBuffer *messageBuffer;

#define MAX_RESPONSE (10 * 1024)

typedef struct arguments {
    GtkWidget * arg1;
    GtkWidget * arg2;
    GtkWidget * arg3;
    GtkWidget * arg4;
    GtkWidget * arg5;
}Arguments;

struct ChatRoom {
    char * name;
    int amountOfUsers;
    char *users[1000];
    char *messages[10000];
};
ChatRoom chatRooms [100];
ChatRoom * currentRoom;


void update_list_rooms();
void update_list_users();

gboolean view_selection_func (GtkTreeSelection *selection,
                              GtkTreeModel     *model,
                              GtkTreePath      *path,
                              gboolean          path_currently_selected,
                              gpointer          userdata);

static GtkWidget *create_list( const char * titleColumn, GtkListStore *model );
static void insert_text( GtkTextBuffer *buffer, const char * initialText );
static void sendButtonClicked(GtkWidget *widget, gpointer data);
static GtkWidget *create_text( const char * initialText , bool editable);
static void createRoomClicked(GtkWidget *widget, gpointer data);
static void trytoSignup(GtkWidget *widget, gpointer data);
static void trytoCreateRoom(GtkWidget *widget, gpointer data);
void showSignupWindow();
static void createAccountClicked(GtkWidget *widget, gpointer data);
//static void signinButtonClicked(GtkWidget *widget, gpointer data);
void showMainWindow();
static void trytoSignin(GtkWidget *widget, gpointer data);
void showLoginWindow();
int open_client_socket(char * host, int port);
int sendCommand(char *  host, int port, char * command, char * response);
void* messageThread();
