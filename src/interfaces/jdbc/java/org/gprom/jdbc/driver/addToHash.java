package org.gprom.jdbc.driver;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Scanner;
import java.util.Set;

// REMEMBER... THE KEY IS ALWAYS UNIQUE!! 
public class addToHash {
	static ArrayList<String> valueCheck = new ArrayList<String>();
	static ArrayList<Integer> keyCheck = new ArrayList<Integer>();

	static Boolean flag = false;

	public static void addhash(int foo, String s) { //For adding SCN and TupleVersion, just change from user input to input values from the table and this function
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
			checkHashMapforQuery.go(foo, s);
		}

	}

}
