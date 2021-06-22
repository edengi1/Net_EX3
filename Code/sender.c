#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <sys/stat.h>
#include <stdlib.h> 
#include <errno.h> 
#include <string.h> 
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h> 

#define SERVER_PORT          5060
#define SERVER_IP_ADDRESS    "127.0.0.1" //local host
#define FILENAME             "data.txt" 
#define FULL_SIZE 1048576
#define BUFF_SIZE 1500 // כמות הבתים

int main() {

    /* INIT FIELDS
    * --> file_pointer == a pointer to the given file .
    * --> server_address == a structure describing an internet socket address.
    */    
    char buffer[BUFF_SIZE]; // מערך של 1500 בתים
    int file_size; // מצהירים על משתנה
    FILE *file_pointer;
    socklen_t length;

    int number_of_runs = 0; 
    int i = 0;
    while(i < 2) { 

        int j = 0;
        while(j < 5) {
            /* CREATE CLIENT SOCKET 
            * --> SOCK_STREAM == a TCP protocol type.
            */
            int client_socket = socket(AF_INET, SOCK_STREAM, 0); // יצירת סוקט מסוג TCP
            if(client_socket == -1) { // אם נכשל
                fprintf(stderr, "Couldn't create the socket : %s\n", strerror(errno)); // להדפיס ולצאת מהתוכנית
                exit(EXIT_FAILURE); // failing exit status.
            }

            /* CONGESTION CONTROL PROGRAM
            * --> getsockopt == function manipulates options associated with a socket. // פונקציה שמשמשת לאפשרויות שמקושרות לסוקט
            * --> setsockopt == function shall set the option specified by the option_name argument. // פונקציה שמגדירה או קובעת את האפשרות שמצויינת על ידי הארגומנט
            * --> IPPROTO_TCP == to indicate that an option is interpreted by the TCP. // מציין את האפשרות שמתפרש על ידי הTCP
            * --> TCP_CONGESTION == Congestion control algorithm.
            */
            int get_sock_opt = getsockopt(client_socket, IPPROTO_TCP, TCP_CONGESTION, buffer, &length); //קיבלנו את האופשיינס הדיפולטיבי
            if( get_sock_opt != 0) {// בודקים אם נכשל
                perror("getsockopt");
                exit(EXIT_FAILURE); // failing exit status.
            }
            if(i == 0) {
                strcpy(buffer,"cubic"); //// מעתיק לתוך באפק את המילה קיוביק
            } else {
                strcpy(buffer,"reno");// מעתיק לתוך באפר את רנו
            }
            length = sizeof(buffer);
            int set_sock_opt = setsockopt(client_socket, IPPROTO_TCP, TCP_CONGESTION, buffer, length);// אחרי שנכנסנו לבאפר דורסים את המידע הקודם ומכניסים לקליינט סוקט את מה שהיה בתוך הבאפר
            if(set_sock_opt !=0 ) { // אם נכשל
                perror("setsockopt");
                exit(EXIT_FAILURE); // failing exit status.
            }
            get_sock_opt = getsockopt(client_socket, IPPROTO_TCP, TCP_CONGESTION, buffer, &length);// נבדוק שוב פעם שהמידע נכתב כמו שצריך 
            if( get_sock_opt != 0) {
                perror("getsockopt");
                exit(EXIT_FAILURE); // failing exit status.
            }
            number_of_runs++; //עכשיו יהיה 1
            printf("\n======= (%d) Current CC: %s  ====== \n",number_of_runs, buffer); //הבאפק מחזיק או קיוביק או רנו
            /* construct the server_address struct
            * --> memset == zeroing the server_address struct.
            * --> AF_INET == IPv4 type.
            * --> htons == short, network byte order converter.
            * --> inet_pton == convert the IP address from String type.
	    rval- return value 
            */
            struct sockaddr_in server_address; // מגדירים סרבר
            memset(&server_address, 0, sizeof(server_address)); //אתחול של סטראקצר , מאתחלים הכל באפסים
            server_address.sin_family = AF_INET; // משהו קבוע
            server_address.sin_port = htons(SERVER_PORT); //מוציא חבילה החוצה דואגים שזה יהיה בפורמט מסויים באופן אחיד כל פקטה שיוצאת צריכה להיות מהוסט לנטוורק
            int rval = inet_pton(AF_INET, (const char*)SERVER_IP_ADDRESS, &server_address.sin_addr); // מגדירים את האיפי אדרס של השרת שזה לוקאל הוסט(הגדרה של כתובת אי פי)
            if(rval <= 0) { // בודקים את הריטרנ ווליו אם הצליח או לא
                printf("inet_pton() failed");
                return -1;
            }

            /* CONNECT TO THE SERVER
            * --> make a connection to the server with client_socket.
            */
            int connection = connect(client_socket, (struct sockaddr *) &server_address, sizeof(server_address)); //בקונקט יש סוקט של קליינט שמכאן הקליינט התחבר אלינו
            if(connection == -1) { 
                fprintf(stderr, "connect() failed with error code --> %s\n", strerror(errno));
                exit(EXIT_FAILURE); // failing exit status.
            } 
            else { // אם הקונקט הצליח
                printf("connected to server!\n");
            }

            /* SEND DATA TO SERVER */
            int numbytes = recv(client_socket, buffer, BUFF_SIZE, 0); // מקבלים מתוך הסוקט  את כמות הבתים שקראנו לתוך הבאפר, כמות הבתים זה הבאפ סייז שזה 1500
        	if (numbytes == -1) { 
            		perror("recv");
            		exit(1);
		    }	

	        buffer[numbytes] = '\0'; // דואגים שהתו האחרון  בתוך הבאפר יהיה סלאש 0

        	printf("Received from server: '%s' \n", buffer);

            file_pointer = fopen(FILENAME, "r"); // אחרי שהמידע בתוך הבאפר פותחים קובץ לקריאה(שמו דאטא)
            if(file_pointer == NULL) { // בודקים אם הצלחנו לפתוח
                fprintf(stderr, "Failed to open file 1mb.txt : %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            }

            int data_stream;
            int size = 0;
            while( ( data_stream = fread(buffer,1,sizeof(buffer),file_pointer) ) > 0 ) { // כל פעם קוראים 1500 בתים כגודל הבאפר
                size += send(client_socket, buffer, data_stream, 0);// שולחים את המידע (כמות הבתים שנשלחו)
            }

            if(size == FULL_SIZE) { //אם הגענו לפול סייז
                printf("successfully sent 1MB file: %d\n",size); // הכל נשלח כמו שצריך
            }else {
                printf("sadly sent just %d out of %d\n",size,FULL_SIZE);// כשל
            }
            sleep(1);// ישנים במשך שנייה
            close(client_socket);// סוגרים את הסוקט
            j++;// מגדילים את J
        }
        i++;
    }
    return 0;
}//פתחנו סוקט הגדרנו את קונפיגורציית הסידור של הסוקט, 

//אחרי ששלפנו את המידע עשינו סט ראינו או קיוביק או רנו ואז אמרנו בוא נרים סרבר ואז הגדלנו את המספר ההרצות הדפסנו אחרי זה קוגרשן קונטרולה

// הגדרנו סרבר אחרי שהתחברנו מה שקיבלנו שולחים בחזרה

//ברגע שקיבלנו פותחים קובץ קוראים את כל התוכן של הקובץ בקבוצות של 1500 בתים ובסוף לאחר שממלאין את הבאפר ב1500 בתים אנחנו מיד שולחים , בסןף מגיעים לסכום מסויים

// בודקים את הסכום שווה לגודל הקובץ שזה 1 מגה אם כן מושלם אם לא נכשל והולכים לישון סוגרים את הפייל דיסקרקטור וממשיכים הלאה לקריאה הבאה
