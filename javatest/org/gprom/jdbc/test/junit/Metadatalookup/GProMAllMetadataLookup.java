/**
 * 
 */
package org.gprom.jdbc.test.junit.Metadatalookup;

import junit.framework.JUnit4TestAdapter;

import org.gprom.jdbc.test.junit.Metadatalookup.TestPostgresMetadataLookup;
import org.junit.runner.RunWith;
import org.junit.runners.Suite;

/**
 * @author lord_pretzel
 *
 */
@RunWith(Suite.class)
@Suite.SuiteClasses({
	//TestPostgresMetadataLookup.class,
	TestOracleMetadataLookup.class
	})
public class GProMAllMetadataLookup {

}

