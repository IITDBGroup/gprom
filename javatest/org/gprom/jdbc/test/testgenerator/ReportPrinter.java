package org.gprom.jdbc.test.testgenerator;

import java.util.Vector;

import org.junit.Test;

import junit.framework.TestCase;

public class ReportPrinter extends TestCase {

	public ReportPrinter () {
		
	}
	
	@Test
	public void testPrintReport () {
		Vector<String> marked;
		
		marked = TestInfoHolder.getInstance().getMarkedErrors();
		
		System.out.println("******************************");
		System.out.println(" marked as error ");
		System.out.println("******************************");
		
		for (int i = 0; i < marked.size(); i++) {
			System.out.println(marked.get(i));
		}
	}
	
}
