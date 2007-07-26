#ifndef WIDGETTOOLTIP_H_
#define WIDGETTOOLTIP_H_

#include "../MessageMapPolicyClasses.h"
#include "../aspects/AspectFont.h"
#include "../aspects/AspectRaw.h"
#include "../aspects/AspectVisible.h"
#include "../xCeption.h"

namespace SmartWin {

class WidgetToolTip :
	public MessageMapPolicy< Policies::Subclassed >,
	
	// Aspects
	public AspectEnabled< WidgetToolbar >,
	public AspectFont< WidgetToolbar >,
	public AspectRaw< WidgetToolbar >,
	public AspectVisible< WidgetToolbar >
{
	struct Dispatcher
	{
		typedef std::tr1::function<void (LPNMTTDISPINFO)> F;

		Dispatcher(const F& f_) : f(f_) { }

		HRESULT operator()(private_::SignalContent& params) {
			f(reinterpret_cast< LPNMTTDISPINFO >( params.Msg.LParam ));
			return 0;
		}

		F f;
	};
	
public:
	/// Class type
	typedef WidgettoolTip ThisType;

	/// Object type
	typedef ThisType * ObjectType;

	typedef MessageMapPolicy<Policies::Subclassed> PolicyType;

	class Seed
		: public SmartWin::Seed
	{
	public:
		typedef WidgetToolTip::ThisType WidgetType;

		//TODO: put variables to be filled here

		/// Fills with default parameters
		// explicit to avoid conversion through SmartWin::CreationalStruct
		explicit Seed();

		/// Doesn't fill any values
		Seed( DontInitialize )
		{}
	};

	/// Default values for creation
	static const Seed & getDefaultSeed();

	/// Actually creates the Toolbar
	/** You should call WidgetFactory::createToolbar if you instantiate class
	  * directly. <br>
	  * Only if you DERIVE from class you should call this function directly.       
	  */
	virtual void create( const Seed & cs = getDefaultSeed() );

protected:
	// Constructor Taking pointer to parent
	explicit WidgetToolTip( SmartWin::Widget * parent );

	// To assure nobody accidentally deletes any heaped object of this type, parent
	// is supposed to do so when parent is killed...
	virtual ~WidgetToolTip()
	{}

}

inline WidgetToolTip::Seed::Seed()
{
	* this = WidgetToolTip::getDefaultSeed();
}

inline WidgetToolTip::WidgetToolTip( SmartWin::Widget * parent )
	: PolicyType( parent )
{
	// Can't have a text box without a parent...
	xAssert( parent, _T( "Can't have a ToolTip without a parent..." ) );
}

}

#endif /*WIDGETTOOLTIP_H_*/
