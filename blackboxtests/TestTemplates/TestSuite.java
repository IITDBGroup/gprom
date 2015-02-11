package PACKAGE;

import org.junit.runner.RunWith;
import org.junit.runners.Suite;
import org.perm.testgenerator.ReportPrinter;
import junit.framework.JUnit4TestAdapter;

@RunWith(Suite.class)
@Suite.SuiteClasses({
		CHILDREN
	})
public class NAME {

	public static junit.framework.Test suite() {
		return new JUnit4TestAdapter(NAME.class);
	}
}