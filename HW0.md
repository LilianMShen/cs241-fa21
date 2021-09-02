1)
#include <unistd.h>

int main() {
	write(1, "Hi! My name is Lilian", 21);
	return 0;
}

2)
#include <unistd.h>

void write_triangle(int n) {
	int row, i;
	for (row = 0; row < n; row++) {
		for (i = 0; i <= row; i++) {
			write (STDERR_FILENO, "*", 1);
		}
		write(STDERR_FILENO, "\n", 1);
	}
}

3)
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
	// permissions to read and write
	mode_t mode = S_IRUSR | S_IWUSR;
	int files = open("hello_world.txt", O_CREAT | O_TRUNC | O_RDWR, mode);
	write(files, "Hi! My name is Lilian", 21);
	close(files);
	
	return 0;
}

4)
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main() {
	// permissions to read and write
	mode_t mode = S_IRUSR | S_IWUSR;
	int files = open("hello_world.txt", O_CREAT | O_TRUNC | O_RDWR, mode);
	if (files == -1) {
		perror("open failed");
		exit(1);
	}
	printf("Hi! My name is Lilian");
	// write(files, "Hi! My name is Lilian", 21);
	close(files);
	
	return 0;
}

5) Write is a system call while printf is a standard library function. Write only lets you write a sequence of bytes while printf lets you write data in many different formats (like using %d). System calls are costly. (Full disclosure, I read an explanation from stack overflow before typing my answer, because I was unsure myself). 


Chapter 2
1) Typically there are 8 bits in a byte, but this is not always the case. Actually, I am still confused about this.

2) There is 1 byte in a char but 8 bits in a byte in a char.

3) 1 byte in a char, 2 bytes in a short, 4 bytes in an int and a long, 8 bytes in a double and a long long

4) The address would be 0x7fbd9d48 because you add 4 bytes every time you + 1 to data (because each int is 4 bytes, so you’re incrementing by 4 bytes every time).

5) data[3] can be rewritten as 3[data]. It simply looks at the memory address (and it is the same)

6) It segfaults because pointers are read only, so writing to one will cause the segfault.

7) str_size is equal to 12.

8) str_len is equal to 5.

9) X = “abc”


