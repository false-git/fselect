#ifndef __FSELECT_H
#define __FSELECT_H

#include <memory>

/*!
 * \brief fselectが所属するnamespace
 */
namespace wl {
    class fselect_private;
    /*!
     * \brief select(2)のwrapperクラス
     *
     * select(2)を呼び出すためのwrapperクラスです。
     *
     * read_watch()/write_watch()/except_watch()で監視するfdを登録した後、
     * select()で無限待ちに入ります。
     * select()は、監視対象のfile descriptorが指定した状態になったときに戻ります。
     * select()の終了後、read_isready()/write_isready()/except_isready()
     * の呼び出しにより、fdの状態を調べることができます。
     *
     * select()の呼び出し中に中断したい場合は、別スレッドからstop()を呼び出します。
     * このとき、select()の引数にstop()呼び出しによって中断したことが返ります。
     *
     * select(2)と違い、select()の呼び出しによって監視対象は変化しません。
     * このため、select()の完了後、再度select()を呼び出すと、同じ監視対象を
     * 再度監視します。
     * 監視をやめる場合は、read_unwatch()/write_unwatch()/except_unwatch()を呼び出します。
     *
     * 一般的な使い方は、IOスレッドによりselect()を呼び出し、UIスレッド等の
     * 別スレッドからstop()を呼び出すことを想定しています。
     *
     * 以下の場合の動作は保証されません。
     * * select()実行中にwatch/unwatchで監視対象を変更したとき
     * * select()を複数スレッドから同時に実行したとき
     * * 監視対象のfile descriptorをclose()したとき
     */
    class fselect {
    public:
	/*!
	 * \brief コンストラクタ
	 */
	fselect();
	/*!
	 * \brief デストラクタ
	 */
	virtual ~fselect();
	/*!
	 * \brief オブジェクトが有効かどうかを返す。
	 * @return 有効なときtrue
	 */
	bool is_valid() const;
	/*!
	 * \brief select(2)を実行する。
	 * @param is_stop [OUT] stop()の呼び出しにより終了するときtrue
	 * @return select(2)の返値
	 */
	int select(bool &is_stop);
	/*!
	 * \brief selectの実行を中断する。
	 */
	void stop();
	/*!
	 * \brief read監視をやめる(FD_CLR相当)
	 * @param fd 監視をやめるfile descriptor。省略すると全ての監視をやめる。
	 */
	void read_unwatch(int fd = -1);
	/*!
	 * \brief read監視しているかどうかを返す(FD_ISSET相当)
	 * @param fd 調べるfile descriptor
	 * @return read監視対象のときtrue
	 */
	bool read_iswatch(int fd) const;
	/*!
	 * \brief read可能になったかどうかを返す(FD_ISSET相当)
	 * @param fd 調べるfile descriptor
	 * @return read可能なときtrue
	 */
	bool read_isready(int fd) const;
	/*!
	 * \brief read可能になったかどうかを監視する(FD_SET相当)
	 * @param fd 監視するfile descriptor
	 */
	void read_watch(int fd);
	/*!
	 * \brief write監視をやめる(FD_CLR相当)
	 * @param fd 監視をやめるfile descriptor。省略すると全ての監視をやめる。
	 */
	void write_unwatch(int fd = -1);
	/*!
	 * \brief write監視しているかどうかを返す(FD_ISSET相当)
	 * @param fd 調べるfile descriptor
	 * @return write監視対象のときtrue
	 */
	bool write_iswatch(int fd) const;
	/*!
	 * \brief write可能になったかどうかを返す(FD_ISSET相当)
	 * @param fd 調べるfile descriptor
	 * @return write可能なときtrue
	 */
	bool write_isready(int fd) const;
	/*!
	 * \brief write可能になったかどうかを監視する(FD_SET相当)
	 * @param fd 監視するfile descriptor
	 */
	void write_watch(int fd);
	/*!
	 * \brief except監視をやめる(FD_CLR相当)
	 * @param fd 監視をやめるfile descriptor。省略すると全ての監視をやめる。
	 */
	void except_unwatch(int fd = -1);
	/*!
	 * \brief except監視しているかどうかを返す(FD_ISSET相当)
	 * @param fd 調べるfile descriptor
	 * @return except監視対象のときtrue
	 */
	bool except_iswatch(int fd) const;
	/*!
	 * \brief except が発生したかどうかを返す(FD_ISSET相当)
	 * @param fd 調べるfile descriptor
	 * @return exceptが発生したときtrue
	 */
	bool except_isready(int fd) const;
	/*!
	 * \brief exceptが発生したかどうかを監視する(FD_SET相当)
	 * @param fd 監視するfile descriptor
	 */
	void except_watch(int fd);
    private:
	std::unique_ptr<fselect_private> d;

	fselect(fselect const&) = delete;
	fselect& operator=(fselect const&) = delete;
	fselect(fselect &&) = delete;
	fselect& operator=(fselect &&) = delete;
    };

}

#endif /* __FSELECT_H */
