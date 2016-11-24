/**
 * 
 */
package org.gprom.jdbc.test.testgenerator.dataset;

import java.util.Arrays;

/**
 * @author lord_pretzel
 *
 */
public class TableCompartor {

	public static final TableCompartor inst = new TableCompartor ();
	
	public static TableCompartor getInst () {
		return inst;
	}
	
	public boolean compareTableAgainstMany (DBTable actual, DBTable[] expected) {
		for(DBTable e: expected)
			if (actual.equals(e))
				return true;
		return false;
	}
	
}
