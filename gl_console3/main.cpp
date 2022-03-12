#include "src/console.h"

#include <iostream>


int main() {

	glfwInit();

	Console con;

	con.create();

	while(con) {
		
		glfwPollEvents();

	}

	glfwTerminate();

	return 0;
}