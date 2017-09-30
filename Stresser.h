class Stresser {
public:
	virtual void setLoad(double load) = 0;
	virtual void stopLoad() = 0;
};