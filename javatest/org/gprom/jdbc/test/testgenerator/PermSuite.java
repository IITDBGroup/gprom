package org.gprom.jdbc.test.testgenerator;

import java.util.Vector;

public class PermSuite {

	private PermSuite parent;
	private Vector<PermSuite> children;
	private String name;
	private String className;
	
	public PermSuite (String name) {
		this.name = name;
		className = name.replaceAll("\\.", "_");
		children = new Vector<PermSuite> ();
	}

	public PermSuite getParent() {
		return parent;
	}

	public void setParent(PermSuite parent) {
		this.parent = parent;
	}

	public Vector<PermSuite> getChildren() {
		return children;
	}

	public void addChild (PermSuite child) {
		if (!children.contains(child)) {
			children.add(child);
		}
	}
	
	public void addChildWithDupes (PermSuite child) {
		children.add(child);
	}
	
	public void setChildren(Vector<PermSuite> children) {
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
		PermSuite child;
		
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
