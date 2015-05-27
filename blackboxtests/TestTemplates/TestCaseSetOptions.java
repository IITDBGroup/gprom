package PACKAGE;

import java.sql.SQLException;

import org.junit.Test;
import org.gprom.jdbc.test.testgenerator.AbstractGProMTester;
import org.gprom.jdbc.test.testgenerator.OptionsManager;

public class NAME extends AbstractGProMTester {

	public NAME (String name) {
		super (name);
	}
	
	@Test
    public void testOptions () throws SQLException, Exception {
		OptionsManager.getInstance().setOptions (SETTING);
    }
	
}