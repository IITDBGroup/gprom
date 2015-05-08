package org.gprom.jdbc.test.testgenerator;

import java.util.Vector;

public class GProMSuite {

	private GProMSuite parent;
	private Vector<GProMSuite> children;
	private String name;
	private String className;
	
	public GProMSuite (String name) {
		this.name = name;
		className = name.replaceAll("\\.", "_");
		children = new Vector<GProMSuite> ();
	}

	public GProMSuite getParent() {
		return parent;
	}

	public void setParent(GProMSuite parent) {
		this.parent = parent;
	}

	public Vector<GProMSuite> getChildren() {
		return children;
	}

	public void addChild (GProMSuite child) {
		if (!children.contains(child)) {
			children.add(child);
		}
	}
	
	public void addChildWithDupes (GProMSuite child) {
		children.add(child);
	}
	
	public void setChildren(Vector<GProMSuite> children) {
		this.children = children;
	}

	public String getName() {
		return name;
	}

	public String getFileName () {
		String fileName;
		
		fileName = name.replaceAll("\\_(\\d)*", ""); 
		return fileName;
	}
	
	public String getClassName () {
		return className;
	}
	
	public void setName(String name) {
		this.name = name;
	}
	
	public String getClassText () {
		StringBuffer result;
		GProMSuite child;
		
		result = new StringBuffer();
		
		for (int i = 0; i < children.size() - 1; i++) {
			child = children.get(i);
			result.append(child.getClassName() + ".class,\n");
		}
		child = children.get(children.size() - 1);
		result.append(child.getClassName() + ".class");
		
		return result.toString();
	}
	
	
}
