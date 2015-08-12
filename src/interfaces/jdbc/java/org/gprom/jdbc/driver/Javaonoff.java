package org.gprom.jdbc.driver;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.Scanner;

class Javaonoff implements MyInterface {
	public void method1() {
		System.out.println("java on");
		try (PrintWriter out = new PrintWriter(new BufferedWriter(
				new FileWriter("java.txt", true)))) {
			out.print("on");
		} catch (IOException e) {
			// exception handling left as an exercise for the reader
		}
	}

	public void method2() {
		System.out.println("java off");
		try (PrintWriter out = new PrintWriter(new BufferedWriter(
				new FileWriter("java.txt", true)))) {
			out.print("off");
		} catch (IOException e) {
			// exception handling left as an exercise for the reader
		}
	}

	public void method3() {
		System.out.println("Property not set");
	}

	public void method4() {
		System.out.println("Should be in format \"java -\"");
	}

	public static void main(String args[]) {
		Scanner in = new Scanner(System.in);
		String java = in.nextLine();
		File file = new File("java.txt");
		file.delete();
		
		MyInterface obj = new Javaonoff();
		if (!java.contains("java -")) {
			obj.method4();
		} else {
			if (java.contains("on")) {
				obj.method1();
			} else if (java.contains("off")) {
				obj.method2();
			} else {
				obj.method3();
			}

		}
	}
}
