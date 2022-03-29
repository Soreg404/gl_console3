#include "src/console.h"

#include <iostream>


int main() {

	glfwInit();

	Console con;
	con.create(400, 600);

	while(con) {
		
		glfwPollEvents();

	}

	glfwTerminate();

	return 0;
}