/**
 * 
 */
package org.gprom.jdbc.test.testgenerator.dataset;

import java.util.List;
import java.util.Set;

/**
 * @author lord_pretzel
 *
 */
public interface DBTable {

	public String[] getColumnNames();
	public int getNumCols();
	public Set<Row> getRows();
	public List<Row> getRowList();
	public boolean isOrdered();
	public void setOrdered(boolean isOrdered);
	public boolean hasRow(Row row);
	public int getNumRows();
}
