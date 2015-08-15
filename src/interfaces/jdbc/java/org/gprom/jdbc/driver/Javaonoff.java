package org.gprom.jdbc.driver;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.PrintWriter;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Scanner;
import java.util.Set;

public class Javaonoff implements MyInterface {
	public void method1() {
		System.out.println("java on");
		try (PrintWriter out = new PrintWriter(new BufferedWriter(
				new FileWriter("java.txt", true)))) {
			out.print("on");
		} catch (IOException e) {
			
		}
	}

	public void method2() {
		System.out.println("java off");
		try (PrintWriter out = new PrintWriter(new BufferedWriter(
				new FileWriter("java.txt", true)))) {
			out.print("off");
		} catch (IOException e) {
			
		}
	}

	public void method3() {
		System.out.println("Property not set");
	}

	public void method4() {
		System.out.println("Should be in format \"java -\"");
	}
	public void method5(String s){
		try (PrintWriter out = new PrintWriter(new BufferedWriter(
				new FileWriter("queries.txt", true)))) {
			out.print(s);
			out.print("\n");

		} catch (IOException e) {
			
		}
	}
	public void method6(String s){
		try (PrintWriter out = new PrintWriter(new BufferedWriter(
				new FileWriter("queries.csv", true)))) { ////I have also stored the data in csv since Tanu and Alex mentioned that they use CSV files for transforming data
			out.print(s);
			out.print("\n");

		} catch (IOException e) {
			
		}
	}
	public void method7(String s){
		try (PrintWriter out = new PrintWriter(new BufferedWriter(
				new FileWriter("results.txt", true)))) { 
			out.print(s);

			out.print("\n");

		} catch (IOException e) {
			
		}
	}
	public void method8(String s){
		try (PrintWriter out = new PrintWriter(new BufferedWriter(
				new FileWriter("results.csv", true)))) { //I have also stored the data in csv since Tanu and Alex mentioned that they use CSV files for transforming data
			out.print(s);

			out.print("\n");

		} catch (IOException e) {
			
		}
	}
	public void method9(int key, String value){
		HashMap<Integer, String> hmap = new HashMap<Integer, String>();
		HashMap<Integer, String> map = null;
		ArrayList<String> valueList = new ArrayList<String>();
		ArrayList<Integer> keyList = new ArrayList<Integer>();

		try {
			FileInputStream fis = new FileInputStream("hashmap.ser");
			ObjectInputStream ois = new ObjectInputStream(fis);
			map = (HashMap) ois.readObject();
			ois.close();
			fis.close();
		} catch (IOException ioe) {
			ioe.printStackTrace();
			return;
		} catch (ClassNotFoundException c) {
			System.out.println("Class not found");
			c.printStackTrace();
			return;
		}
		Set set = map.entrySet();
		Iterator iterator = set.iterator();
		while (iterator.hasNext()) {
			Map.Entry mentry = (Map.Entry) iterator.next();
			int sss = Integer.parseInt("" + mentry.getKey());
			String sString = "" + mentry.getValue();

			valueList.add(sString);
			keyList.add(sss);

		}

		for (int i = 0; i < valueList.size(); i++) {
			// System.out.println(valueList.get(i));

			hmap.put(keyList.get(i), valueList.get(i));
		}
		hmap.put(key, value);
		try {
			FileOutputStream fos = new FileOutputStream("hashmap.ser");
			ObjectOutputStream oos = new ObjectOutputStream(fos);
			oos.writeObject(hmap);
			oos.close();
			fos.close();
			System.out
					.printf("Serialized HashMap data is saved in hashmap.ser");
			Javaonoff obj = new Javaonoff();
			obj.method11();
		} catch (IOException ioe) {
			ioe.printStackTrace();
		}
	}
	public void method10(int foo, String s){
		ArrayList<String> valueCheck = new ArrayList<String>();
		ArrayList<Integer> keyCheck = new ArrayList<Integer>();
		Javaonoff obj = new Javaonoff();
		Boolean flag = false;
		 //For adding SCN and TupleVersion, just change from user input to input values from the table and this function
		Scanner in = new Scanner(System.in);
		//System.out.println("Enter Int Key");
		//String s1 = in.nextLine();
		//int foo = Integer.parseInt(s1);
		//System.out.println("Enter String Value");
		//String s = in.nextLine();
		/*int foo = 79178265;			// dummy SCN
		String s = "AAAR3sAAEAAAACXABB";	// dummy rowid   // 
*/		HashMap<Integer, String> map = null;
		try {
			FileInputStream fis = new FileInputStream("hashmap.ser");
			ObjectInputStream ois = new ObjectInputStream(fis);
			map = (HashMap) ois.readObject();
			ois.close();
			fis.close();
		} catch (IOException ioe) {
			ioe.printStackTrace();
			return;
		} catch (ClassNotFoundException c) {
			System.out.println("Class not found");
			c.printStackTrace();
			return;
		}
		Set set = map.entrySet();
		Iterator iterator = set.iterator();
		System.out.println("\nCURRENT............>!!!!!!!!!!!!!!!!\n");
		while (iterator.hasNext()) {
			Map.Entry mentry = (Map.Entry) iterator.next();

			System.out.print("key: " + mentry.getKey() + " & Value: ");
			System.out.println(mentry.getValue());

			int sss = Integer.parseInt("" + mentry.getKey());
			String sString = "" + mentry.getValue();

			keyCheck.add(sss);
			valueCheck.add(sString);
		}
		for (int i = 0; i < keyCheck.size(); i++) {
			if (keyCheck.get(i).equals(foo) && valueCheck.get(i).equals(s)) {
				System.out.println("\n\nStatement Exists");
				flag = true;
			}
		}
		if (!flag) {
			System.out.println("\nStatement does not exist");
			System.out.println("\nNEW............>!!!!!!!!!!!!!!!!\n");
			obj.method9(foo, s);
		}
	}
	public void method11(){
		HashMap<Integer, String> map = null;
		try {
			FileInputStream fis = new FileInputStream("hashmap.ser");
			ObjectInputStream ois = new ObjectInputStream(fis);
			map = (HashMap) ois.readObject();
			ois.close();
			fis.close();
		} catch (IOException ioe) {
			ioe.printStackTrace();
			return;
		} catch (ClassNotFoundException c) {
			System.out.println("Class not found");
			c.printStackTrace();
			return;
		}
		Set set = map.entrySet();
		Iterator iterator = set.iterator();
		while (iterator.hasNext()) {
			Map.Entry mentry = (Map.Entry) iterator.next();
			System.out.print("\nkey: " + mentry.getKey() + " & Value: "
					+ mentry.getValue());
		}
	}
	public static void javaLogging(String s) {
		String java = s;
		File file = new File("java.txt");
		file.delete();
	
		Javaonoff obj = new Javaonoff();
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
	public static void QueryStoretxt(String s)
	{
		Javaonoff obj = new Javaonoff();
		obj.method5(s);
	}
	public void QueryStorecsv(String s)
	{
		Javaonoff obj = new Javaonoff();
		obj.method6(s);	
	}
	public void ResultStoretxt(String s)
	{
		Javaonoff obj = new Javaonoff();
		obj.method7(s);
	}
	public void ResultStorecsv(String s)
	{
		Javaonoff obj = new Javaonoff();
		obj.method8(s);
	}
	public void checkHash(int key, String value)
	{
		Javaonoff obj = new Javaonoff();
		obj.method9(key, value);
	}
	public void addHash(int key, String value){

		Javaonoff obj = new Javaonoff();
		obj.method10(key, value);
	}

	
}
