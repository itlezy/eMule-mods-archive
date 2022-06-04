#pragma once

// blocklist��һ�鲻�ص���block�����򼯺�
// {
//		[begin,end),
//		[begin,end),
//		[begin,end),
//		...
// }

class blocklist
{
public:
	blocklist()	{	}
	~blocklist()	{	}

public:
	typedef pair<uint64_t, uint64_t>	block_t;
	typedef vector<block_t>				blocks_t;
public:
	// ���[begin,end)...�Զ������ص�
	void insert(const uint64_t begin, const uint64_t end);
	void insert(const vector<block_t>& blockvector);
	void insert(const blocklist& blocklist);

	// ɾ��[begin,end)...
	void erase(const uint64_t begin, const uint64_t end);
	void erase(const vector<block_t>& blockvector);
	void erase(const blocklist& blocklist);

	// ȡ[begin,end)��Χ�ڵ�blocklist�Ӽ�
	void subset(const uint64_t begin, const uint64_t end, vector<block_t>& blockvector) const;
	void subset(const uint64_t begin, const uint64_t end, blocklist& blocklist) const;

	// ��������block�����ܳ��ȣ�TODO: ����������ã���Ҫ��һ���Ż���
	uint64_t size(void) const;

	// ���/�Ƿ�Ϊ��
	void clear(void);
	bool empty(void) const;
	
	bool in(const uint64_t pos,block_t& range);

	bool intersection(const block_t& range);
	bool intersection(const uint64_t begin, const uint64_t end)
	{
		block_t range;
		range.first = begin;
		range.second = end;
		return intersection(range);
	}

public:
	blocks_t				m_blocklist;

	////////////////////////////////////////////////////////////////////////
	// �����ǲ��Դ���
public:
	void dump(string& dump) const;
};
