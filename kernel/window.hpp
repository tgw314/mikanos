#pragma once

#include <optional>
#include <vector>

#include "frame_buffer.hpp"
#include "frame_buffer_config.hpp"
#include "graphics.hpp"

/** Window クラスはグラフィックの表示領域を表す
 *
 * タイトルやメニューがあるウィンドウだけでなく,
 * マウスカーソルの表示領域なども対象とする
 */
class Window {
   public:
    // WindowWriter は Window と関連付けられた PixelWriter を提供する
    class WindowWriter : public PixelWriter {
       public:
        WindowWriter(Window &window) : window_{window} {}
        // 指定された位置に指定された色を描く
        virtual void Write(Vector2D<int> pos, const PixelColor &c) override {
            window_.Write(pos, c);
        }
        // Width は関連付けられた Window の横幅をピクセル単位で返す
        virtual int Width() const override { return window_.Width(); }
        // Height は関連付けられた Window の高さをピクセル単位で返す
        virtual int Height() const override { return window_.Height(); }

       private:
        Window &window_;
    };

    // 指定されたピクセル数の平面描画領域を作成する
    Window(int width, int height, PixelFormat shadow_format);
    ~Window() = default;
    Window(const Window &rhs) = delete;
    Window &operator=(const Window &rhs) = delete;

    /** 与えられた FrameBuffer にこのウィンドウの表示領域を描画する。
     *
     * dst  描画先
     * position  writer の左上を基準とした描画位置
     */
    void DrawTo(FrameBuffer &dst, Vector2D<int> position);
    // 透過色を設定する
    void SetTransparentColor(std::optional<PixelColor> c);
    // このインスタンスに紐付いた WindowWriter を取得する
    WindowWriter *Writer();

    // 指定した位置のピクセルを返す
    const PixelColor &At(Vector2D<int> pos) const;
    // 指定した位置のピクセルを書き込む
    void Write(Vector2D<int> pos, PixelColor c);

    // 平面描画領域の横幅をピクセル単位で返す
    int Width() const;
    // 平面描画領域の高さをピクセル単位で返す
    int Height() const;

    /** このウィンドウの平面描画領域内で，矩形領域を移動する。
     *
     * src_pos   移動元矩形の原点
     * src_size  移動元矩形の大きさ
     * dst_pos   移動先の原点
     */
    void Move(Vector2D<int> dst_pos, const Rectangle<int> &src);

   private:
    int width_, height_;
    std::vector<std::vector<PixelColor>> data_{};
    WindowWriter writer_{*this};
    std::optional<PixelColor> transparent_color_{std::nullopt};

    FrameBuffer shadow_buffer_{};
};
