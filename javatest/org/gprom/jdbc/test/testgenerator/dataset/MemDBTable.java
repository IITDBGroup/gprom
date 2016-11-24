/**
 * 
 */
package org.gprom.jdbc.test.testgenerator.dataset;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.HashSet;
import java.util.List;
import java.util.Set;

/**
 * @author lord_pretzel
 *
 */
public class MemDBTable implements DBTable {

	private List<String> columnNames;
	private Set<Row> rows;
	private boolean isOrdered = false;
	
	public MemDBTable () {
		columnNames = new ArrayList<String> ();
		rows = new HashSet<Row> ();
		isOrdered = false;
	}
	
	
	
	/* (non-Javadoc)
	 * @see org.gprom.jdbc.test.testgenerator.dataset.DBTable#getColumnNames()
	 */
	@Override
	public String[] getColumnNames() {
		return columnNames.toArray(new String[0]);
	}

	/* (non-Javadoc)
	 * @see org.gprom.jdbc.test.testgenerator.dataset.DBTable#getNumCols()
	 */
	@Override
	public int getNumCols() {
		return columnNames.size();
	}

	/* (non-Javadoc)
	 * @see org.gprom.jdbc.test.testgenerator.dataset.DBTable#getRows()
	 */
	@Override
	public Set<Row> getRows() {
		return rows;
	}

	/* (non-Javadoc)
	 * @see org.gprom.jdbc.test.testgenerator.dataset.DBTable#hasRow(java.lang.String[])
	 */
	@Override
	public boolean hasRow(Row row) {
		return rows.add(row);
	}

	/* (non-Javadoc)
	 * @see org.gprom.jdbc.test.testgenerator.dataset.DBTable#getNumRows()
	 */
	@Override
	public int getNumRows() {
		return rows.size();
	}

	/* (non-Javadoc)
	 * @see org.gprom.jdbc.test.testgenerator.dataset.DBTable#getRowList()
	 */
	@Override
	public List<Row> getRowList() {
		return new ArrayList<Row> (rows);
	}

	/* (non-Javadoc)
	 * @see org.gprom.jdbc.test.testgenerator.dataset.DBTable#isOrdered()
	 */
	@Override
	public boolean isOrdered() {
		return isOrdered;
	}



	public void setColumnNames(List<String> columnNames) {
		this.columnNames = columnNames;
	}

	public void addColumn (String col) {
		this.columnNames.add(col);
	}
	
	public void addRow (Row r) {
		rows.add(r);
	}
	
	public void setRows(Set<Row> rows) {
		this.rows = rows;
	}

	public void setOrdered(boolean isOrdered) {
		this.isOrdered = isOrdered;
	}
	
	public int hashValue () {
		int result = columnNames.hashCode();
		result |= rows.hashCode();
		return result;
	}
	
	public boolean equals (Object o) {
		if (!(o instanceof DBTable))
			return false;
		
		DBTable t2 = (DBTable) o;
		if (!Arrays.equals(getColumnNames(),t2.getColumnNames()))
			return false;
		if (isOrdered != t2.isOrdered())
			return false;
		if (isOrdered) {
			if (!getRowList().equals(t2.getRowList()))
				return false;
		}
		else {
			if (!getRows().equals(t2.getRows()))
				return false;
		}
		return true;
	}
	
	public String toString() {
		StringBuilder r = new StringBuilder();
		
		r.append("|");
		for(String c: columnNames) {
			r.append(" " + c + " |");
		}
		r.append("\n");
		
		for(Row row: rows) {
			r.append(row.toString());
			r.append("\n");
		}
		return r.toString();
	}

}