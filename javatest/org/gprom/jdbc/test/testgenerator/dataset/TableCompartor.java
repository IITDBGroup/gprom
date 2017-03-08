/**
 * 
 */
package org.gprom.jdbc.test.testgenerator.dataset;

import java.util.Arrays;

import org.apache.log4j.Logger;

/**
 * @author lord_pretzel
 *
 */
public class TableCompartor {

	Logger log = Logger.getLogger(TableCompartor.class);
	
	public static final TableCompartor inst = new TableCompartor ();
	
	public static TableCompartor getInst () {
		return inst;
	}
	
	public boolean compareTableAgainstMany (DBTable actual, DBTable[] expected) {
		log.info("table is:\n\n" + actual 
				+ "\nexpected was on of\n\n"  + Arrays.toString(expected));
		for(DBTable e: expected)
			if (actual.equals(e))
				return true;
		return false;
	}
	
}
