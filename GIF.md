# generating GIF from terminal output

```sh
❯ asciinema rec -c build/src/blotgl output.asciinema
❯ agg --theme asciinema --fps-cap 10 --font-family 'Terminess Nerd Font Mono' --font-size 10 --line-height 1 --renderer fontdue  output.asciinema output.gif
```
