import sys
import serial
#import os
import time
import datetime
import MySQLdb
from time import strftime

def insertDataToDB(data_read):
	db = MySQLdb.connect(host="localhost", user="root", passwd="zazaza123", db="water_level")
	cur = db.cursor()
	rssi_data = 0

	while True:
		#rssi_data = 0;
		print data_read
		
		temp_data = data_read.split(",")[0] // TEMP
		humid_data = data_read.split(",")[1] // HUMIDITY
		rssi_data = data_read.split(",")[2] // RSSI
		snr_data = data_read.split(",")[3] // SNR
		water_level_data = data_read.split(",")[4] // WATER LEVEL
		water_temp_data = data_read.split(",")[5] // WATER TEMP
		voltage_data = data_read.split(",")[6] // VOLTAGE

		print "Water Level: " + water_level_data
		print "Temperature Water: " + water_temp_data
		print "Voltage Probe: " + voltage_data
		print "Temperate Box: " + temp_data
		print "Humidity Box: " + humid_data
		print "RSSI: " + rssi_data
		print "SNR: " + snr_data

		#if(data_read < 0): 
		#	rssi_data = data_read
		#else:
		#	water_level_data =  data_read
		
		datetimeWrite = (time.strftime("%Y-%m-%d ") + time.strftime("%H:%M:%S"))
		#sql_query = ("""INSERT INTO water_level_data (datetime,water_level,rssi) VALUES (%s,%s,%s)""",(datetimeWrite,water_level_data,rssi_data))
		sql_query = ("""INSERT INTO monitoring_system (datetime, temp, humid,  rssi, snr, water_level, water_temp, voltage) VALUES (%s, %s, %s, %s, %s, %s, %s, %s)""",(datetimeWrite, temp_data, humid_data, rssi_data, snr_data, water_level_data, water_temp_data, voltage_data))
		
		try:
        		print "Writing to database..."
        		# Execute the SQL command
        		cur.execute(*sql_query)
        		# Commit your changes in the database
        		db.commit()
        		print "Write Complete"
 
    		except:
        		# Rollback in case there is any error
        		db.rollback()
        		print "Failed writing to database"
 
    		cur.close()
    		db.close()
    		break

def main():
	serial_conn = serial.Serial("/dev/ttyACM0", 9600, timeout=1)
        data_sum = ""
        data_read = ""

        while True:
                data_read = serial_conn.read(2000)
                if data_read != '':
			#print(data_read)
			insertDataToDB(data_read)
                sys.stdout.flush()
                time.sleep(5)
	#insertDataToDB(data_read)

if __name__ == "__main__":
	main()
