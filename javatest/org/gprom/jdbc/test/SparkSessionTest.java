/**
 * 
 */
package org.gprom.jdbc.test;

import java.util.ArrayList;
import java.util.List;

import org.apache.spark.sql.Dataset;
import org.apache.spark.sql.Row;
import org.apache.spark.sql.SparkSession;



/**
 * @author lord_pretzel
 *
 */
public class SparkSessionTest {

	public static void main (String[] args) {
		SparkSession spark = SparkSession.builder().appName("mytestapp").master("spark://borismacbook:56655").getOrCreate();
				
//		
//		class Person {
//			String name;
//			int age;
//			
//			public Person (String name, int age) {
//				this.name = name;
//				this.age = age;
//			}
//		}
//		
//		List<Person> persons = new ArrayList<Person> ();
//		
//		persons.add(new Person("Peter", 12));
//		persons.add(new Person("Bob", 32));
		String url = "jdbc:gprom:postgresql://127.0.0.1:5432/gpromtest";
//		Dataset df = spark.createDataFrame(persons, Person.class);
		Dataset df = spark.read().jdbc(url, "r", null);
		df.createOrReplaceGlobalTempView("persons");
		
		
		Dataset<Row> res = spark.sql("SELECT * FROM persons;");
		res.printSchema();
		res.collect();
		
	}
	
}
