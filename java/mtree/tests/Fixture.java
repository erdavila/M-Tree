package mtree.tests;

import java.io.BufferedReader;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

class Fixture {
	
	static class Action {
		char cmd;
		Data data;
		Data queryData;
		double radius;
		int limit;

		private Action(char cmd, Data data, Data queryData, double radius, int limit) {
			this.cmd = cmd;
			this.data = data;
			this.queryData = queryData;
			this.radius = radius;
			this.limit = limit;
		}
	}


	private int dimensions;
	List<Action> actions;

	
	static String path(String fixtureName) {
		return "cpp/tests/fixtures/" + fixtureName + ".txt";
	}

	static Fixture load(String fixtureName) {
		String fixtureFileName = path(fixtureName);
		BufferedReader fixtureFile = null;
		try {
			fixtureFile = new BufferedReader(new FileReader(fixtureFileName));
			
			int dimensions = Integer.parseInt(fixtureFile.readLine());
			
			int count = Integer.parseInt(fixtureFile.readLine());
			
			Fixture fixture = new Fixture(dimensions);
			
			for(int i = 0; i < count; i++) {
				String line = fixtureFile.readLine();
				List<String> fields = new ArrayList<String>(Arrays.asList(line.split("\\s+")));
				
				char cmd = fields.remove(0).charAt(0);
				Data data = fixture.readData(fields);
				Data queryData = fixture.readData(fields);
				double radius = Double.parseDouble(fields.remove(0));
				int limit = Integer.parseInt(fields.remove(0));
			
				fixture.actions.add(new Action(cmd, data, queryData, radius, limit));
				
				assert fields.isEmpty();
			}
			
			return fixture;
		} catch (IOException e) {
			throw new RuntimeException(e);
		} finally {
			try {
				fixtureFile.close();
			} catch (IOException e) {
				throw new RuntimeException(e);
			}
		}
	}
	
	private Fixture(int dimensions) {
		this.dimensions = dimensions;
		this.actions = new ArrayList<Action>();
	}
	

	private Data readData(List<String> fields) {
		int[] values = new int[dimensions];
		for(int d = 0; d < dimensions; d++) {
			values[d] = Integer.parseInt(fields.remove(0));
		}
		return new Data(values);
	}
}
