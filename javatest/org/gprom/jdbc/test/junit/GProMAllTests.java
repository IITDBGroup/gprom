/**
 * 
 */
package org.gprom.jdbc.test.junit;

import junit.framework.JUnit4TestAdapter;

import org.gprom.jdbc.test.junit.Metadatalookup.GProMAllMetadataLookup;
import org.junit.runner.RunWith;
import org.junit.runners.Suite;

/**
 * @author lord_pretzel
 *
 */
@RunWith(Suite.class)
@Suite.SuiteClasses({
	GProMAllMetadataLookup.class
	})
public class GProMAllTests {

	public static junit.framework.Test suite() {
		return new JUnit4TestAdapter(GProMAllTests.class);
	}
}

