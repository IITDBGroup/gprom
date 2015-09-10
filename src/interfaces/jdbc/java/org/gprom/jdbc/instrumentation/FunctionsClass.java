package org.gprom.jdbc.instrumentation;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileWriter;
import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.PrintWriter;
import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import com.google.common.collect.ArrayListMultimap;
import com.google.common.collect.Multimap;

public class FunctionsClass {
	static void javaLogging(String s) {
		String java = s;
		File file = new File("java.txt");
		file.delete();

		if (!java.contains("java -")) {
			System.out.println("Should be in format \"java -\"");
		} else {
			if (java.contains("on")) {
				System.out.println("java on");
				try (PrintWriter out = new PrintWriter(new BufferedWriter(
						new FileWriter("java.txt", true)))) {
					out.print("on");
				} catch (IOException e) {

				}
			} else if (java.contains("off")) {
				System.out.println("java off");
				try (PrintWriter out = new PrintWriter(new BufferedWriter(
						new FileWriter("java.txt", true)))) {
					out.print("off");
				} catch (IOException e) {

				}
			} else {
				System.out.println("Property not set");
			}
		}
	}

	static void queryLogger(String s) {
		try (PrintWriter out = new PrintWriter(new BufferedWriter(
				new FileWriter("queries.txt", true)))) {
			out.print(s);
			out.print("\n");

		} catch (IOException e) {

		}
		try (PrintWriter out = new PrintWriter(new BufferedWriter(
				new FileWriter("queries.csv", true)))) {
			out.print(s);
			out.print("\n");

		} catch (IOException e) {

		}
	}

	static void resultLogger(String s) {
		try (PrintWriter out = new PrintWriter(new BufferedWriter(
				new FileWriter("results.txt", true)))) {
			out.print(s);

			out.print("\n");

		} catch (IOException e) {

		}
		try (PrintWriter out = new PrintWriter(new BufferedWriter(
				new FileWriter("results.csv", true)))) {
			out.print(s);

			out.print("\n");

		} catch (IOException e) {

		}
	}
	static void HashLogger(String k, String v) throws FileNotFoundException, IOException, ClassNotFoundException{
		File f = new File("hashmap1.ser");
		if (f.exists() && !f.isDirectory()) {
			ObjectInputStream in = new ObjectInputStream(new FileInputStream(
					"hashmap1.ser"));
			Object obj = in.readObject();
			HashMap<String, List<String>> map = (HashMap<String, List<String>>) obj;

			List<String> valSet = new ArrayList<String>();
			try {
				valSet.addAll(map.get(k));
				valSet.add(v);
				map.put(k, valSet);
			} catch (Exception e) {

				valSet.add(v);
				map.put(k, valSet);
			}

			FileOutputStream fos = new FileOutputStream("hashmap1.ser");
			ObjectOutputStream oos = new ObjectOutputStream(fos);
			oos.writeObject(map);
			oos.close();
			fos.close();

			displayAll();
		} else {
			Map<String, List<String>> map = new HashMap<String, List<String>>();
			List<String> valSetOne = new ArrayList<String>();
			valSetOne.add("RowId");
			map.put("SCN", valSetOne);
			FileOutputStream fos = new FileOutputStream("hashmap1.ser");
			ObjectOutputStream oos = new ObjectOutputStream(fos);
			oos.writeObject(map);
			oos.close();
			fos.close();
		}
	}

	static void displayAll() throws ClassNotFoundException, IOException {
		ObjectInputStream in = new ObjectInputStream(new FileInputStream(
				"hashmap1.ser"));
		Object obj = in.readObject();
		HashMap<String, List<String>> map = (HashMap<String, List<String>>) obj;
		System.out
				.println("Fetching Keys and corresponding [Multiple] Values \n");
		for (Map.Entry<String, List<String>> entry : map.entrySet()) {
			String key = entry.getKey();
			List<String> values = entry.getValue();

			System.out.println("Key = " + key);
			System.out.println("Values = " + values + "\n");
		}

	}
 static void printResult(ResultSet rs, String s) throws SQLException {
		
		
		try (PrintWriter out = new PrintWriter(new BufferedWriter(
				new FileWriter("myfile.txt", true)))) {

			
			Javaonoff.getInstance().QueryLogger(s);
			
			Javaonoff.getInstance().JavaLogger("-------------------------------------------------------------------------------");
			
			Javaonoff.getInstance().resultsLogger("-------------------------------------------------------------------------------");
			Javaonoff.getInstance().resultsLogger("\n");
			for (int i = 1; i <= rs.getMetaData().getColumnCount(); i++)
				Javaonoff.getInstance().JavaLogger(rs.getMetaData().getColumnLabel(i) + "\t|");
				
			for (int i = 1; i <= rs.getMetaData().getColumnCount(); i++)
			// out.print(rs.getMetaData().getColumnLabel(i) + "\t|");
			{
				Javaonoff.getInstance().resultsLogger(rs.getMetaData().getColumnLabel(i) + "\t|");
			}
			Javaonoff.getInstance().JavaLogger("\n");
			Javaonoff.getInstance().resultsLogger("\n");
			
			Javaonoff.getInstance().JavaLogger("-------------------------------------------------------------------------------");
			
			Javaonoff.getInstance().resultsLogger("-------------------------------------------------------------------------------");
			Javaonoff.getInstance().resultsLogger("\n");

			while (rs.next()) {
				for (int i = 1; i <= rs.getMetaData().getColumnCount(); i++)
					Javaonoff.getInstance().JavaLogger(rs.getString(i) + "\t|");
					
				for (int i = 1; i <= rs.getMetaData().getColumnCount(); i++)

				{
					Javaonoff.getInstance().resultsLogger(rs.getString(i) + "\t|");
					Javaonoff.getInstance().resultsLogger("\n");
				}
				Javaonoff.getInstance().JavaLogger("\n");
			}
			Javaonoff.getInstance().JavaLogger("\n");
			Javaonoff.getInstance().resultsLogger("\n");

			Javaonoff.getInstance().JavaLogger("-------------------------------------------------------------------------------");
			
			Javaonoff.getInstance().resultsLogger("-------------------------------------------------------------------------------");
			Javaonoff.getInstance().JavaLogger("\n");
			Javaonoff.getInstance().resultsLogger("\n");
			Javaonoff.getInstance().JavaLogger("\n");
			Javaonoff.getInstance().resultsLogger("\n");

		} catch (IOException e) {
			// exception handling left as an exercise for the reader
		}
		
	}

}
