/**
 * 
 */
package org.gprom.jdbc.test.testgenerator.dataset;

import java.util.Arrays;
import java.util.List;

/**
 * @author lord_pretzel
 *
 */
public class Row {

	private final String[] values;
	
	public Row (String[] values) {
		this.values = values;
	}
	
	public Row (Row o) {
		this.values = Arrays.copyOf(o.values, o.values.length);
	}
	
	public Row (List<String> r) {
		values = r.toArray(new String[0]);
	}
	
	public int hashCode() {
		return Arrays.hashCode(values);
	}
	
	public boolean equal (Object o) {
		if (!(o instanceof Row))
			return false;
		Row other = (Row) o;
		return Arrays.equals(this.values, other.values);
	}
	
	public String toString() {
		StringBuilder result = new StringBuilder();
		result.append("|");
		for(String c: values) {
			result.append(" ");
			result.append(c);
			result.append(" |");
		}
		return result.toString();
	}
}
