class ShutterInformation {
	public:
		void cmdStop( unsigned long millis ) {
			if ( state == E_IDLE )
				return;
			
			position = getPosition( millis );
			
			state = E_IDLE;
			time_last_command = millis;
			
			position_acquiring = 0;
		}
		
		void cmdUp( unsigned long millis ) {
			if ( state == E_OPENING )
				return;
			
			position = getPosition( millis );
			
			state = E_OPENING;
			time_last_command = millis;
		}
		
		void cmdDown( unsigned long millis ) {
			if ( state == E_CLOSING )
				return;
			
			position = getPosition( millis );
			
			state = E_CLOSING;
			time_last_command = millis;
		}

		// 0 open, 100 completely closed
		int getPosition( unsigned long millis ) {
			int pos_temp = 0;
			
			switch ( state ) {
				case E_IDLE:
					pos_temp = position;
					break;
				case E_OPENING:
					if ( millis - time_last_command > time_full_change )
						pos_temp = 0;
					else
						pos_temp = position - (millis - time_last_command) * 100 / time_full_change;
					break;
				case E_CLOSING:
					if ( millis - time_last_command > time_full_change )
						pos_temp = 100;
					else
						pos_temp = position + (millis - time_last_command) * 100 / time_full_change;
					break;
			}
			
			if ( pos_temp > 100 )
				pos_temp = 100;
			if ( pos_temp < 0 )
				pos_temp = 0;
			
			return pos_temp;
		}
		
		/* set the desired position in percentage (0 open). does not toggle a CC1101 command, but
			returns:
			0 if engine is IDLE and position is within +- ?%, nothing todo
			1 if engine is DRIVING but position is within +- ?%, shutter needs to stop
			2 if shutter needs to go up
			3 if shutter needs to go down
		*/
		int setPosition( unsigned long millis, unsigned int pos_set ) {
			
			if ( positionInRange ( millis, pos_set, 3 ) ) {
				if ( state == E_IDLE ) {
					return 0;
				} else {
					return 1;
				}
			}
			
			position_acquiring = 1;
			
			int pos_cur = getPosition ( millis );
			
			if ( pos_set > pos_cur ) {
				position_desired = pos_set;
				return 3;
			}
			
			// pos_set < pos_cur
			position_desired = pos_set;
			return 2;			
		}
		
		// returns 1 (once) if a stop command should be issued
		int shouldSendStop ( unsigned long millis ) {
			if ( state == E_IDLE ) {
				position_acquiring = 0;
				return 0;
			}
			
			if ( !position_acquiring )
				return 0;
			
			if ( positionInRange ( millis, position_desired, 3 ) ) {
				return 1;
			}
			
			return 0;
		}
		
		void setShutterTime( unsigned long time ) {
			if ( time <= 0 )
				return;
			
			time_full_change = time;
		}
		
		unsigned long getShutterTime() {
			return time_full_change;
		}
		
	private:
		typedef enum {
			E_IDLE,
			E_CLOSING,
			E_OPENING
		} state_t;
		
		state_t state = E_IDLE;
		int position;
		int position_desired;
		int position_acquiring = 0;
		
		unsigned long time_last_command;
		unsigned long time_full_change = 20800;	//for labour room; WZ door 31000
		
		// returns 1 if this shutter is in the desired position, CC1101 should send STOP
		// range 'cause this is not realtime and might miss some percentage
		// positions 0 and 100 will be checked accurate
		int positionInRange ( unsigned long millis, int pos_desired, int range ) {
					
			int pos_cur = getPosition ( millis );
			
			if ( pos_desired == 0 ) {
				if ( pos_cur == 0 )
					return 1;
			} else if ( pos_desired == 100 ) {
				if ( pos_cur == 100 )
					return 1;
			} else {
				if ( pos_cur > pos_desired - range && pos_cur < pos_desired + range ) {
					return 1;
				}
			}
			
			return 0;
		}
};

ShutterInformation shutter_information[16];
