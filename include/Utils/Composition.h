#if !defined(__CLONE_FROM_GOOGLE_CHROME_COMPOSITION_TEXT_H_)
#define __CLONE_FROM_GOOGLE_CHROME_COMPOSITION_TEXT_H_

#include <string>
#include <vector>
namespace DuiLib
{
	template<typename theElement>
	class IList
	{
	public:
		virtual theElement*  at(unsigned int) = 0;
		virtual unsigned int count()     = 0;
		virtual unsigned int getType()   = 0;
		virtual theElement* getBegin()   = 0;
		virtual theElement* getNext()    = 0;
		virtual void  reset()            = 0;
		virtual void  clear()			 = 0;
		virtual unsigned int erase(theElement*)= 0;
		virtual void  release()		     = 0;
	};
	class Locker
	{
	public:
		void Lock(){}
		void UnLock(){}
	};
	template< typename theItem,class theLock=Locker >
	class IListImpl : public IList<theItem>
	{
	public:
		IListImpl(unsigned int nType = 0) : m_unType(nType)
		{	m_nSz = 0;		}
		virtual ~IListImpl()
		{	clear();		}
		theItem* push_back()
		{
			m_lock.Lock();
			theItem* pI = new theItem;
			if( pI )
				m_lstC.push_back( pI );
			m_lock.UnLock();
			return pI;
		}
		theItem*	 getBegin()
		{
			m_lock.Lock();
			if( m_lstC.empty() )
			{
				m_lock.UnLock();
				return 0;
			}
			m_nSz = 0;
			m_lock.UnLock();
			return m_lstC[0];
		}
		theItem*	 getNext()
		{
			theItem *pRet = 0;
			m_lock.Lock();
			if( m_nSz < m_lstC.size() )
			{
				pRet = m_lstC[m_nSz];
				m_nSz++;
				m_lock.UnLock();
				return pRet;
			}
			m_lock.UnLock();
			return 0;
		}
		theItem*  at(unsigned int unV)
		{
			m_lock.Lock();
			if( 0 == m_lstC.size() )
			{
				m_lock.UnLock();
				return 0;
			}
			if( unV < m_lstC.size())
			{
				m_lock.UnLock();
				return m_lstC.at(unV);
			}
			m_lock.UnLock();
			return 0;
		}
		void         reset()
		{	
			m_lock.Lock();
			m_nSz = 0;	
			m_lock.UnLock();
		}
		unsigned int count()
		{
			m_lock.Lock();
			unsigned int sz = m_lstC.size();	
			m_lock.UnLock();
			return sz;
		}
		unsigned int erase(theItem* pI)
		{
			m_lock.Lock();
			std::vector<theItem*>::iterator it = std::find(m_lstC.begin(),m_lstC.end(),pI);
			if( it != m_lstC.end() )
			{
				delete (*it);
				m_lstC.erase(it);
				m_nSz = 0;
			}
			m_lock.UnLock();
			return m_lstC.size();
		}
		void clear()
		{
			m_lock.Lock();
			if(m_lstC.size())
			{
				for(m_nSz=0;m_nSz < m_lstC.size();m_nSz++)
				{
					if( m_lstC[m_nSz] )
					{
						delete m_lstC[m_nSz];
						m_lstC[m_nSz] = 0;
					}
				}
				m_lstC.clear();
			}
			m_nSz = 0;
			m_lock.UnLock();
		}
		void  release()
		{	delete this;		}
		unsigned int getType(void)
		{	return m_unType;	}
	private:
		std::vector<theItem*> m_lstC;
		size_t				  m_nSz;
		unsigned int		  m_unType;
		theLock               m_lock;
	};

	struct CompositionText;
	// A Range contains two integer values that represent a numeric range, like the
	// range of characters in a text selection. A range is made of a start and end
	// position; when they are the same, the Range is akin to a caret. Note that
	// |start_| can be greater than |end_| to respect the directionality of the
	// range.
	class Range {
	public:
		// Creates an empty range {0,0}.
		Range();
		// Initializes the range with a start and end.
		Range(size_t start, size_t end);

		// Initializes the range with the same start and end positions.
		explicit Range(size_t position);
		// The |total_length| paramater should be used if the CHARRANGE is set to
		// {0,-1} to indicate the whole range.
		Range(const CHARRANGE& range, LONG total_length = -1);
		// Returns a range that is invalid, which is {size_t_max,size_t_max}.
		static const Range InvalidRange();
		// Checks if the range is valid through comparision to InvalidRange().
		bool IsValid() const;
		// Getters and setters.
		size_t start() const { return start_; }
		void set_start(size_t start) { start_ = start; }
		size_t end() const { return end_; }
		void set_end(size_t end) { end_ = end; }
		// Returns the absolute value of the length.
		size_t length() const 
		{
			ptrdiff_t length = end() - start();
			return length >= 0 ? length : -length;
		}
		bool is_reversed() const { return start() > end(); }
		bool is_empty() const { return start() == end(); }
		// Returns the minimum and maximum values.
		size_t GetMin() const;
		size_t GetMax() const;
		bool operator==(const Range& other) const;
		bool operator!=(const Range& other) const;
		Range&	operator=(const Range& other)
		{
			this->start_ = other.start();
			this->end_   = other.end();
			return (*this);
		}
		bool EqualsIgnoringDirection(const Range& other) const;
		// Returns true if this range intersects the specified |range|.
		bool Intersects(const Range& range) const;
		// Returns true if this range contains the specified |range|.
		bool Contains(const Range& range) const;
		// Computes the intersection of this range with the given |range|.
		// If they don't intersect, it returns an InvalidRange().
		// The returned range is always empty or forward (never reversed).
		Range Intersect(const Range& range) const;
		CHARRANGE ToCHARRANGE() const;
	private:
		size_t start_;
		size_t end_;
	};
	// Intentionally keep sync with WebKit::WebCompositionUnderline defined in:
	// third_party/WebKit/Source/WebKit/chromium/public/WebCompositionUnderline.h
	struct CompositionUnderline
	{
		CompositionUnderline()
			: start_offset(0),end_offset(0),color(0),thick(false) 
		{

		}
		CompositionUnderline(unsigned s, unsigned e,COLORREF c, bool t)
			: start_offset(s),
			end_offset(e),
			color(c),
			thick(t) 

		{

		}
		bool operator==(const CompositionUnderline& rhs) const
		{
			return (this->start_offset == rhs.start_offset) &&
				(this->end_offset == rhs.end_offset) &&
				(this->color == rhs.color) &&
				(this->thick == rhs.thick);
		}
		bool operator!=(const CompositionUnderline& rhs) const 
		{
			return !(*this == rhs);
		}
		// Though use of unsigned is discouraged, we use it here to make sure it's
		// identical to WebKit::WebCompositionUnderline.
		unsigned start_offset;
		unsigned end_offset;
		COLORREF    color;
		bool		thick;
	};
	typedef std::vector<CompositionUnderline> CompositionUnderlines;
	// A struct represents the status of an ongoing composition text.
	struct CompositionText 
	{
		CompositionText() : beginPos_(-1),m_bAvaliable(false)
		{

		}
		virtual ~CompositionText()
		{

		}
		bool operator==(const CompositionText& rhs) const 
		{
			if ((this->text != rhs.text) ||
				(this->selection != rhs.selection) ||
				(this->underlines.size() != rhs.underlines.size()) ||
				(this->beginPos_ != rhs.beginPos_) ||
				(this->m_bAvaliable != rhs.m_bAvaliable)
				)
				return false;
			for (size_t i = 0; i < this->underlines.size(); ++i) 
			{
				if (this->underlines[i] != rhs.underlines[i])
					return false;
			}
			return true;
		}
		bool operator!=(const CompositionText& rhs) const 
		{
			return !(*this == rhs);
		}
		CompositionText operator=(const CompositionText &rhs )
		{
			this->text		= rhs.text;
			this->selection = rhs.selection;
			this->underlines= rhs.underlines;
			this->beginPos_ = rhs.beginPos_;
			this->m_bAvaliable= rhs.m_bAvaliable;
			return (*this);
		}
		void Clear()
		{
			text.clear();
			underlines.clear();
			selection = Range();
			beginPos_ = -1;
		}
		// Content of the composition text.
		std::wstring text;
		// Underline information of the composition text.
		// They must be sorted in ascending order by their start_offset and cannot be
		// overlapped with each other.
		CompositionUnderlines underlines;
		// Selection range in the composition text. It represents the caret position
		// if the range length is zero. Usually it's used for representing the target
		// clause (on Windows). Gtk doesn't have such concept, so background color is
		// usually used instead.
		Range selection;
		//起始位置
		size_t beginPos_;
		//活动标记
		bool	m_bAvaliable;
	};
	typedef IListImpl<CompositionText>	CompositionTexts;	//输入词队列
};
#endif //__CLONE_FROM_GOOGLE_CHROME_COMPOSITION_TEXT_H_