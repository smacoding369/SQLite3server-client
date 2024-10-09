#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sqlite3.h>

#define PORT 8080

// دالة مساعدة للتحقق من الأخطاء
void handle_error(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}


int main() {
    int server_fd, new_socket;
    //int valread;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // إنشاء سوكت (Socket)
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // وضع خيارات للسوكيت (عادة لا يتم تغييرها)
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    // تعيين عنوان و منفذ للسوكيت
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    // ربط السوكيت بالعنوان والمنفذ
    if (bind(server_fd, (struct sockaddr *)&address, 
            sizeof(address))<0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // جعل السوكيت في وضع الاستماع لطلبات الاتصال
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while(1) {
        // قبول اتصال جديد من عميل
        if ((new_socket = accept(server_fd,(struct sockaddr *)&address,(socklen_t*)&addrlen))<0){
            perror("accept");
            exit(EXIT_FAILURE);
        }
        
		//char *message = "Welcome With S.M.A Login System\n";
		//send(new_socket, message, strlen(message), 0);
		
        // استقبال البيانات من العميل
        // استقبال البيانات من العميل
        char buffer[1024] = {0};
        //memset(buffer, 0, sizeof(buffer)); // تنظيف المصفوفة
        int valread = read( new_socket , buffer, 1024);  // إعادة تعريف valread
        if (valread <= 0) {
            // حدث خطأ أثناء القراءة، قم بمعالجة الخطأ هنا
            perror("read");
            exit(EXIT_FAILURE);
        }

        // تحليل البيانات المستقبلة (نفترض أن البيانات هي "login username password")
        // تحليل البيانات المستقبلة باستخدام strtok_r للحماية من هجمات buffer overflows
        char *saveptr;
        char *command = strtok_r(buffer, " ", &saveptr);
        char *username = strtok_r(NULL, " ", &saveptr);
        char *password = strtok_r(NULL, " ", &saveptr);

        // الاتصال بقاعدة البيانات SQLite
        sqlite3 *db;
        if (sqlite3_open("users.db", &db) != SQLITE_OK) {
            handle_error("Failed to open database");
        }

        // تنفيذ الاستعلام بناءً على الأمر باستخدام عبارات معدة لمنع حقن SQL
        sqlite3_stmt *stmt;
		
		//char sql[100];
        if (strcmp(command, "login") == 0) {
			const char *sql = "SELECT * FROM users WHERE username = ? AND password = ?";
			if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
			handle_error("Failed to prepare statement");
			}
			sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
			sqlite3_bind_text(stmt, 2, password, -1, SQLITE_TRANSIENT);

			int rc = sqlite3_step(stmt);
			if (rc == SQLITE_ROW) {
				// تسجيل الدخول ناجح
				printf("Login successful for user: %s\n", username);
				char *message = "Login successful!\n";
				if (send(new_socket, message, strlen(message), 0) == -1) {
					perror("send");
					// معالجة خطأ الإرسال
				}
			} else {
				// تسجيل الدخول فاشل
				printf("Login failed: Invalid username or password\n");
				char *message = "Login Faild either user or passwd is wrong!\n";
				if (send(new_socket, message, strlen(message), 0) == -1) {
					perror("send");
					// معالجة خطأ الإرسال
				}
			}
		}

		else if (strcmp(command, "register") == 0) {
            const char *sql = "INSERT INTO users (username, password) VALUES (?, ?)";
            if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
                handle_error("Failed to prepare statement");
            }
            sqlite3_bind_text(stmt, 1, username, -1, SQLITE_TRANSIENT);
            sqlite3_bind_text(stmt, 2, password, -1, SQLITE_TRANSIENT);

            if (sqlite3_step(stmt) != SQLITE_DONE) {
                handle_error("Failed to insert user");
            } else {
                send(new_socket, "registration successful\n", strlen("registration successful\n"), 0);
            }
		} 
            
		else if (strcmp(command, "exit") == 0) {
			// إرسال رسالة تأكيد الخروج
			char *message = "Connection closed.\n";
			if (send(new_socket, message, strlen(message), 0) == -1) {
				perror("send");
				// معالجة خطأ الإرسال
			}

			// إغلاق السوكت
			if (close(new_socket) == -1) {
				perror("close");
				// معالجة خطأ إغلاق السوكت
			}
			
			close(new_socket);
			// الخروج من الحلقة
			break;
		} 
			
		else {
				// معالجة أوامر أخرى (إذا كان هناك)
				printf("Invalid command\n");
				char *message = "Invalid command\n";
				send(new_socket, message, strlen(message), 0);
			}

        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }
    return 0;
}
