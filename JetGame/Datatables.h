#ifndef DATATABLES_HEADER
#define DATATABLES_HEADER

#include <map>

//flags for fields
#define FIELD_INTERPOLATE 1//0x1
#define FIELD_INTERPOLATE_ANGLE 32//0x10
#define FIELD_INTERPOLATE_MODULAR 128
#define FIELD_ENTITY 2//0x2
#define FIELD_QUATERNION 4//0x4//not implemented
#define FIELD_NORMAL 8//0x8//networks a normal vector at 1/2 precision and bandwidth, plenty accurate
#define FIELD_ARRAY 16//0x12
#define FIELD_STRING 64
#define FIELD_PREDICTED 128
#define FIELD_ONCHANGE 256
//not really a flag, indicates end of data field list
#define FIELD_END 8888

// using the stringizing operator to save typing...
#define NETF(x) #x,(int)&((entityState_t*)0)->x
#define NETFM(x) #x,(int)&(((currentDTClass*)0)->x)
#define NETFM1(x) (int)&(((currentDTClass*)0)->x)

//this is used to define the data tables for networking various entities
#define ENTITY_DATA_TABLE_START(Name,Class) \
	template <typename T> int ClassInit(T*); \
	namespace Name { \
	struct ignored;\
	};\
	template <> int ClassInit<Name::ignored>(Name::ignored*);\
	namespace Name {\
	int i = ClassInit((Name::ignored*)NULL);\
	} \
	template <> int ClassInit<Name::ignored>(Name::ignored*)\
	{ \
	typedef Class currentDTClass;\
	char* currentDTName = #Name;\
	int offset = ((int)(NetworkedEntity*)(currentDTClass*)1) - 1; \
	static netField_t netf_[] = { \
			{ NETFM(planetid),   1, 0 },\
			{ NETFM(position.x), 4, FIELD_INTERPOLATE, 0 },\
			{ NETFM(position.y), 4, FIELD_INTERPOLATE, 0 },\
			{ NETFM(position.z), 4, FIELD_INTERPOLATE, 0 },\
			{ NETFM(_model_s),   4, FIELD_ONCHANGE, (void*)model_change_callback  }, \
			{ NETFM(health),     2, 0, 0 }

#define ENTITY_DATA_TABLE_START_PREDICTED(Name,Class) \
	template <typename T> int ClassInit(T*); \
	namespace Name { \
	struct ignored;\
		};\
	template <> int ClassInit<Name::ignored>(Name::ignored*);\
	namespace Name {\
	int i = ClassInit((Name::ignored*)NULL);\
		} \
	template <> int ClassInit<Name::ignored>(Name::ignored*)\
		{ \
	typedef Class currentDTClass;\
	char* currentDTName = #Name;\
	int offset = ((int)(NetworkedEntity*)(currentDTClass*)1) - 1; \
	static netField_t netf_[] = { \
						{ NETFM(planetid),   1, 0, 0 },\
						{ NETFM(position.x), 4, FIELD_INTERPOLATE | FIELD_PREDICTED, 0 },\
						{ NETFM(position.y), 4, FIELD_INTERPOLATE | FIELD_PREDICTED, 0 },\
						{ NETFM(position.z), 4, FIELD_INTERPOLATE | FIELD_PREDICTED },\
						{ NETFM(_model_s),   4, FIELD_ONCHANGE, (void*)model_change_callback  }, \
						{ NETFM(health),     2, 0, 0 }

#define ENTITY_DATA_TABLE_START_PREDICTED2(Name,Class,pos) \
	template <typename T> int ClassInit(T*); \
	namespace Name { \
	struct ignored;\
				};\
	template <> int ClassInit<Name::ignored>(Name::ignored*);\
	namespace Name {\
	int i = ClassInit((Name::ignored*)NULL);\
				} \
	template <> int ClassInit<Name::ignored>(Name::ignored*)\
				{ \
	typedef Class currentDTClass;\
	char* currentDTName = #Name;\
	int offset = ((int)(NetworkedEntity*)(currentDTClass*)1) - 1; \
	static netField_t netf_[] = { \
					{ NETFM(planetid),   1, 0, 0 },\
					{ NETFM(position.x), 4, FIELD_INTERPOLATE | FIELD_PREDICTED, (void*)NETFM1(##server_position.x) },\
					{ NETFM(position.y), 4, FIELD_INTERPOLATE | FIELD_PREDICTED, (void*)NETFM1(##server_position.y) },\
					{ NETFM(position.z), 4, FIELD_INTERPOLATE | FIELD_PREDICTED, (void*)NETFM1(##server_position.z) },\
					{ NETFM(_model_s),   4, FIELD_ONCHANGE, (void*)model_change_callback  }, \
					{ NETFM(health), 2, 0, 0}
						
//insert base entity (CEntity) properties here
typedef void callback_t(void*);


//automatically includes data for animated stuff
#define DT_ANIMATED(),\
	/*{ NETFM(anim1),      2, 0 },*/\
			{ NETFM(frame1),     4, FIELD_INTERPOLATE, 0 }//,\
			//{ NETFM(loopanim),   1, 0 }
#define DT_INFO(variable, size) ,\
			{ NETFM(variable), size, 0, 0 }
#define DT_INFOF(variable, size, flag) ,\
			{ NETFM(variable), size, flag, 0 }
#define DT_INFOP(variable, size, flag, pred_dest) ,\
			{ NETFM(variable), size, flag, NETFM1(pred_dest) }
#define DT_ENTITY(variable) ,\
			{ NETFM(variable), 2, FIELD_ENTITY, 0 }
#define DT_ARRAY(variable, size) ,\
			{ NETFM(variable), size, FIELD_ARRAY, 0 }
//only for floats at the moment
#define DT_INTERPOLATED(variable, size) ,\
			{ NETFM(variable), size, FIELD_INTERPOLATE, 0 }
#define DT_INTERPOLATEDF(variable, size, flags) , \
			{ NETFM(variable), size, FIELD_INTERPOLATE | (flags), 0}
//max is just always 1, oh well
#define DT_MODINTERPOLATED(variable) ,\
			{ NETFM(variable), 4, FIELD_INTERPOLATE_MODULAR, 0 } 
//networks a 3 float normal vector as 3 shorts, at a slight precision cost, but saves 1/2 the bandwidth
#define DT_NORMAL(variable) ,\
			{ NETFM(variable), 6, FIELD_NORMAL | FIELD_INTERPOLATE, 0 }

#define DT_STRING(variable) ,\
			{ NETFM(variable), 0, FIELD_STRING, 0 }

#define ENTITY_DATA_TABLE_END() ,\
			{ "end", FIELD_END, FIELD_END, 0, 0}\
		};\
		(*GetDataTable())[GetClassID(currentDTName)] = (netField_t*)&netf_;\
		return 1;\
	};

typedef struct {
	char	*name;
	int	offset;
	int bits;	// 0 = float
	int flags;
	void* data;//optional data for stuffs
} netField_t;

std::map<int, netField_t*>* GetDataTable();

#endif