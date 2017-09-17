package org.gprom.jdbc.testing;

import org.junit.runner.RunWith;
import org.junit.runners.Suite;
import org.junit.runners.Suite.SuiteClasses;

@RunWith(Suite.class)
@SuiteClasses({JSONVersionGraphLoadSave.class, VersionGraphCompose.class })
public class VersionGraphTest {

}
