package PACKAGE;

import java.sql.SQLException;

import org.junit.Test;
import org.gprom.jdbc.test.testgenerator.AbstractGProMTester;
import org.gprom.jdbc.test.testgenerator.ConnectionOptions;
import org.gprom.jdbc.test.testgenerator.OptionsManager;

public class NAME extends AbstractGProMTester {

	public NAME (String name) {
		super (name);
		try {
			ConnectionOptions.getInstance().setPath("PATH");
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
	
	@Test
    public void testSetFile () throws SQLException, Exception {
		setGenerator("FILE");
		OptionsManager.getInstance().setOptions (SETTING);
    }
	
	TESTS
	
}