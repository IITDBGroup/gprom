package org.gprom.jdbc.testing;

import org.junit.runner.RunWith;
import org.junit.runners.Suite;
import org.junit.runners.Suite.SuiteClasses;

@RunWith(Suite.class)
@SuiteClasses({ EdgeTest.class, NodeTest.class, VersionEdgeTest.class,VersionGraphTest.class })
public class AllTests {

}
