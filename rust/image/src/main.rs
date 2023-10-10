use anyhow::Result;

fn main() -> Result<()> {
    let xsz = 600;
    let ysz = 400;
    let mut img = image::RgbImage::new(xsz, ysz);
    for (x, y, p) in img.enumerate_pixels_mut() {
        let r = (x as f32 / xsz as f32 * 255 as f32) as u8;
        let g = (y as f32 / ysz as f32 * 255 as f32) as u8;
        let b = 0;
        *p = image::Rgb([r, g, b]);
    }
    img.save("out.png")?;
    Ok(())
}
