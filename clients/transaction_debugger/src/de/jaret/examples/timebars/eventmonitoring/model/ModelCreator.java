/*
 *  File: ModelCreator.java 
 *  Copyright (c) 2004-2008  Peter Kliem (Peter.Kliem@jaret.de)
 *  A commercial license is available, see http://www.jaret.de.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
package de.jaret.examples.timebars.eventmonitoring.model;

import java.sql.ResultSet;
import java.sql.SQLException;
import java.util.ArrayList;
import java.util.Random;

import de.jaret.util.date.JaretDate;
import de.jaret.util.ui.timebars.model.AddingTimeBarNode;
import de.jaret.util.ui.timebars.model.DefaultHierarchicalTimeBarModel;
import de.jaret.util.ui.timebars.model.DefaultRowHeader;
import de.jaret.util.ui.timebars.model.DefaultTimeBarModel;
import de.jaret.util.ui.timebars.model.DefaultTimeBarNode;
import de.jaret.util.ui.timebars.model.HierarchicalTimeBarModel;
import de.jaret.util.ui.timebars.model.TimeBarModel;
import de.jaret.util.ui.timebars.model.TimeBarNode;
import gprom.gui.*;

/**
 * Simple model creator creating a hierachcial or a flat model and adds some
 * events.
 * 
 * @author kliem
 * @version $Id: ModelCreator.java 801 2008-12-27 22:44:54Z kliem $
 */
public class ModelCreator {

	static Random _random = new Random(12345);

	public static HierarchicalTimeBarModel createHierarchicalModel() {

		JaretDate start = new JaretDate();
		start.setDateTime(1, 1, 2009, 0, 0, 0);
		JaretDate end = new JaretDate();
		end.setDateTime(1, 2, 2009, 0, 0, 0);

		DefaultRowHeader header = new DefaultRowHeader("root");
		TimeBarNode node;
		node = new AddingTimeBarNode(header);

		CollectingTimeBarNode kat1 = new CollectingTimeBarNode(
				new DefaultRowHeader("cat1"));
		node.addNode(kat1);
		CollectingTimeBarNode kat2 = new CollectingTimeBarNode(
				new DefaultRowHeader("Cat2"));
		node.addNode(kat2);
		CollectingTimeBarNode kat3 = new CollectingTimeBarNode(
				new DefaultRowHeader("Cat3"));
		node.addNode(kat3);
		CollectingTimeBarNode kat4 = new CollectingTimeBarNode(
				new DefaultRowHeader("Cat4"));
		node.addNode(kat4);

		// kat1
		EventInterval interval = new EventInterval(start.copy(), end.copy());
		interval.setTitle("long running");
		kat1.addInterval(interval);

		DefaultTimeBarNode n = new DefaultTimeBarNode(new DefaultRowHeader(""));
		interval = new EventInterval(start.copy().advanceHours(10), start
				.copy().advanceHours(15));
		interval.setTitle("short1");
		n.addInterval(interval);
		kat1.addNode(n);

		// kat2
		n = new DefaultTimeBarNode(new DefaultRowHeader(""));
		interval = new EventInterval(start.copy().advanceHours(11), start
				.copy().advanceHours(14));
		interval.setTitle("short2");
		n.addInterval(interval);
		kat2.addNode(n);

		// kat3
		n = new DefaultTimeBarNode(new DefaultRowHeader(""));
		for (int i = 0; i < 20; i++) {
			interval = new EventInterval(start.copy().advanceHours(i * 3),
					start.copy().advanceHours(i * 3 + 1));
			interval.setTitle("short" + i);
			n.addInterval(interval);
		}
		kat3.addNode(n);

		HierarchicalTimeBarModel model = new DefaultHierarchicalTimeBarModel(
				node);

		return model;

	}

	public static TimeBarModel createFlatModel() {

		JaretDate start = new JaretDate();
		start.setDateTime(1, 1, 2009, 0, 0, 0);
		JaretDate end = new JaretDate();
		end.setDateTime(1, 2, 2009, 0, 0, 0);
		//使用sql获取数据库信息
		ResultSet resultSet = DBConnection.getData();

		DefaultTimeBarModel model = new DefaultTimeBarModel();
		TransactionNode node;

		ArrayList<TransactionNode> nodes = new ArrayList<TransactionNode>();
		try {
			while (resultSet.next()) {
//				node = new TransactionNode(resultSet.getString(resultSet
//						.findColumn("LSQLTEXT")), resultSet.getInt(resultSet
//						.findColumn("SCN")),
//						resultSet.getTimestamp("NTIMESTAMP#"));
				
				//添加需要的属性
				//第一个属性找到sql对应的属性名  dbusername换sql对应属性
				node = new TransactionNode(resultSet.getString(resultSet
						.findColumn("SQL_TEXT")), resultSet.getInt(resultSet
						.findColumn("SCN")),
						resultSet.getTimestamp("EVENT_TIMESTAMP"));
				
				nodes.add(node);
			}
		} catch (SQLException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

		for (int i = 0; i < nodes.size(); i++) {
			DefaultRowHeader header = new DefaultRowHeader("SCN "
					+ Integer.toString(nodes.get(i).getSCN()));
			EventTimeBarRow row = new EventTimeBarRow(header);
			
			for (int j = i; j < nodes.size(); j++) {
				if (nodes.get(i).getSCN() == nodes.get(j).getSCN()) {
					start.setDateTime(nodes.get(j).getTimeStamp().getDay(),
							(nodes.get(j).getTimeStamp().getMonth() + 1), nodes
									.get(j).getTimeStamp().getYear() + 1900,
							nodes.get(j).getTimeStamp().getHours(), nodes
									.get(j).getTimeStamp().getMinutes(), 0);
					EventInterval interval = new EventInterval(start.copy()
							.advanceHours(10), start.copy().advanceHours(15));
					
					interval.setTitle(nodes.get(j).getQuery());
					row.addInterval(interval);
					nodes.remove(j);
				}
			}
			model.addRow(row);
		}
		return model;
	}

}
