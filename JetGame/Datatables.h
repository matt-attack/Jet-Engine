#ifndef DATATABLES_HEADER
#define DATATABLES_HEADER

#include <map>

//flags for fields
#define FIELD_INTERPOLATE 1//0x1
#define FIELD_INTERPOLATE_ANGLE 32//0x10
#define FIELD_INTERPOLATE_MODULAR 512
#define FIELD_ENTITY 2//0x2
#define FIELD_QUATERNION 4//0x4//not implemented
#define FIELD_NORMAL 8//0x8//networks a normal vector at 1/2 precision and bandwidth, plenty accurate
#define FIELD_ARRAY 16//0x12
#define FIELD_STRING 64
#define FIELD_PREDICTED 128
#define FIELD_ONCHANGE 256//callback on change
//not really a flag, indicates end of data field list
#define FIELD_END 8888

// using the stringizing operator to save typing...
#define NETF(x) #x,(int)&(((currentDTClass*)0)->x)
#define NETF1(x) (int)&(((currentDTClass*)0)->x)

#define DATA_TABLE_START(Name, Class) \
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
	static networked_field netf_[] = {


//automatically includes data for animated stuff
#define DT_ANIMATED(),\
	/*{ NETFM(anim1),      2, 0 },*/\
			{ NETF(frame1),     4, FIELD_INTERPOLATE, 0 }//,\
			//{ NETFM(loopanim),   1, 0 }
#define DT_INFO(variable, size) ,\
			{ NETF(variable), size, 0, 0 }
#define DT_INFOF(variable, size, flag) ,\
			{ NETF(variable), size, flag, 0 }
#define DT_INFOP(variable, size, flag, pred_dest) ,\
			{ NETF(variable), size, flag, NETF1(pred_dest) }
#define DT_ENTITY(variable) ,\
			{ NETF(variable), 2, FIELD_ENTITY, 0 }
#define DT_ARRAY(variable, size) ,\
			{ NETF(variable), size, FIELD_ARRAY, 0 }
//only for floats at the moment
#define DT_INTERPOLATED(variable, size) ,\
			{ NETF(variable), size, FIELD_INTERPOLATE, 0 }
#define DT_INTERPOLATEDF(variable, size, flags) , \
			{ NETF(variable), size, FIELD_INTERPOLATE | (flags), 0}
//max is just always 1, oh well
#define DT_MODINTERPOLATED(variable) ,\
			{ NETF(variable), 4, FIELD_INTERPOLATE_MODULAR, 0 } 
//networks a 3 float normal vector as 3 shorts, at a slight precision cost, but saves 1/2 the bandwidth
#define DT_NORMAL(variable) ,\
			{ NETF(variable), 6, FIELD_NORMAL | FIELD_INTERPOLATE, 0 }

#define DT_STRING(variable) ,\
			{ NETF(variable), 0, FIELD_STRING, 0 }

#define ENTITY_DATA_TABLE_END() ,\
			{ "end", FIELD_END, FIELD_END, 0, 0}\
		};\
		int i = 0;\
		int p = 0;\
        while (netf_[i].offset != FIELD_END)\
        {\
			p += netf_[i++].bits;\
		}\
        if (p > 128)\
			throw 7;\
		(*GetDataTable())[GetClassID(currentDTName)] = (networked_field*)&netf_;\
		return 1;\
	};


typedef void callback_t(void*);

typedef struct {
	char	*name;
	int	offset;
	int bits;	// 0 = float
	int flags;
	void* data;//optional data for stuffs
} networked_field;

std::map<int, networked_field*>* GetDataTable();

int GetClassID(char* str);

#endif