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
	
	public Row (String ... values) {
		this.values = values;
	}
	
	public Row (Row o) {
		this.values = Arrays.copyOf(o.values, o.values.length);
	}
	
	public Row (List<String> r) {
		values = r.toArray(new String[0]);
	}
	
	public int getNumCols () {
		return values.length;
	}
	
	public String getCol (int i) {
		return values[i];
	}
	
	public int hashCode() {
		return Arrays.hashCode(values);
	}
	
	public boolean equals (Object o) {
		if (!(o instanceof Row))
			return false;
		Row other = (Row) o;
		return Arrays.equals(this.values, other.values);
	}
	
	public String toString() {
		StringBuilder result = new StringBuilder();
		for(String c: values) {
			result.append(" ");
			result.append(c);
			result.append(" |");
		}
		return result.toString();
	}
	
	public String toStringPatched (int[] widths) {
		StringBuilder result = new StringBuilder();
		int i = 0;
		for(String c: values) {
			result.append(" ");
			result.append(MemDBTable.patch(c, widths[i++]));
			result.append(" |");
		}
		return result.toString();
	}
	
}
