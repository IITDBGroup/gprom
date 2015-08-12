package org.gprom.jdbc.driver;

import java.io.*;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;

//REMEMBER... THE KEY IS ALWAYS UNIQUE!! 
public class checkHashMapforQuery {

	public static void go(int key, String value) {
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
			finalInter();
		} catch (IOException ioe) {
			ioe.printStackTrace();
		}
	}

	public static void finalInter() {
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

}