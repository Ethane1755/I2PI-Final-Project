# How to git

#cs 

---

git 就是一種版本控制系統，方便一起做事或是要回推某個版本

### 在電腦安裝git

### 讓git開始偵測資料夾裡面發生的事件

```bash
git init
```

### 把自己的資料夾和Github上面的資料夾同步

``` bash
git pull https://github.com/Ethane1755/I2PI-Final-Project.git
```


**我後來想了一下覺得還是分別創branch好了**

### 建立自己的branch

```bash
git branch your-branch-name //建立，不用引號
git checkout your-branch-name //切到那個branch，不用引號
```

### 在裡面編輯code

請大神帶飛qq

### 把寫好的code丟到Github上面

```bash
git add . //加入所有有改動的檔案
git commit -m "commit message" //簡單講一下改了什麼
git push --set-upstream https://github.com/Ethane1755/I2PI-Final-Project.git your-branch-name //把本地的某branch對應到github上的某branch
```

前面git push後面那一大串一個branch只要用一次，之後都是用

```bash
git push //設定過後就會自動上傳到github上面的那個branch了
```

### 在Github裡面就會看到有Pull Request

就照著他的按鈕按，如果有conflict就在裡面編輯

merge以後也不用刪branch，反正就一個人用一個這樣


### 可能會用到的東西
```bash
git branch //列出所有的branch
git branch -D your-branch-name //把這個branch刪掉
```




