# GitHub 운영 규칙 (Team Git Workflow)

## 1. 목적

본 규칙은 팀 프로젝트에서 **코드 품질 유지, 협업 효율 향상, 작업 이력 관리**를 위해 GitHub 사용 방법을 정의한다.

GitHub는 다음 목적으로 사용한다.

- 소스 코드 버전 관리
- 작업 이슈 관리
- 코드 리뷰
- 문서 관리
- CI 연동

---

## 2. 개발 Workflow

팀 개발은 다음 절차로 진행한다.

```
Issue 생성
   ↓
Branch 생성
   ↓
개발 및 Commit
   ↓
Push
   ↓
Pull Request 생성
   ↓
Code Review
   ↓
Merge → main
```

모든 작업은 **Issue 기반으로 진행한다.**

---

## 3. Issue 관리

모든 작업은 **Issue 생성 후 시작한다.**

### Issue 대상

- 기능 개발
- 버그 수정
- 코드 리팩토링
- 테스트 추가
- 문서 작성
- 기타 유지보수 작업

### Issue Title 규칙

Issue 제목에는 **type을 붙이지 않는다.**

예

```
차일드락 해제 조건 검증 로직 구현
```

작업 유형은 **Label로 관리한다.**

### Issue Template 예시

```
Title
작업 요약

Description
작업 상세 설명

Task
- [ ] 작업1
- [ ] 작업2
```

예

```
Title
차일드락 해제 조건 검증 구현

Description
차량 속도와 후측방 위험 상태를 기반으로
차일드락 해제 가능 여부를 판단하는 로직 구현

Task
- [ ] vehicleSpeed 수신
- [ ] rearSideDanger 수신
- [ ] unlock validation logic 구현
```

---

## 4. Label 규칙

Issue 유형은 Label로 구분한다.

| Label | 의미 |
| --- | --- |
| feature | 새로운 기능 |
| bug | 버그 수정 |
| refactor | 코드 구조 개선 |
| docs | 문서 |
| test | 테스트 |
| chore | 기타 작업 |

---

## 5. Branch 전략

### 기본 Branch

```
main
```

- 항상 **안정적인 코드 유지**
- 직접 push 하지 않고 **PR을 통해서만 반영**

### 작업 Branch

모든 작업 Branch는 **Issue 단위로 생성한다.**

### Branch naming rule

```
feature/{issue-number}-{description}
fix/{issue-number}-{description}
refactor/{issue-number}-{description}
docs/{issue-number}-{description}
test/{issue-number}-{description}
chore/{issue-number}-{description}
```

예

```
feature/23-childlock-validator
fix/41-led-display-error
refactor/30-state-machine
docs/12-update-sequence-diagram
```

### Branch 작성 원칙

- Branch명에는 반드시 **Issue 번호를 포함한다**
- description은 짧고 의미 있게 작성한다
- 공백 대신 를 사용한다

---

## 6. Commit 규칙

### Commit message 형식

```
type: message
```

### Commit type

| type | 의미 |
| --- | --- |
| feat | 새로운 기능 |
| fix | 버그 수정 |
| refactor | 코드 리팩토링 |
| docs | 문서 |
| test | 테스트 |
| style | 코드 스타일 |
| chore | 기타 작업 |

### Commit 예시

```
feat: add child lock validation logic
fix: resolve LED display synchronization issue
refactor: simplify state machine logic
docs: update sequence diagram
```

### Commit 작성 원칙

- 하나의 Commit은 **하나의 의미 있는 변경**만 담는다
- Commit message는 **무엇을 변경했는지 명확히 작성**한다
- 개별 Commit 메시지에는 **Issue 번호를 필수로 포함하지 않는다**
- 불필요한 임시 Commit은 정리 후 Push 한다

---

## 7. Pull Request 규칙

작업 완료 후 반드시 **Pull Request를 통해 main에 merge**한다.

### PR Title 규칙

PR 제목에는 **type을 붙이지 않는다.**

또한 PR 제목에는 **Issue 번호를 포함하지 않는다.**

예

```
Add child lock validation logic
```

### PR 내용

PR 본문에는 반드시 관련 Issue를 연결한다.

기본 형식

```
Description
변경 내용 설명

Closes #이슈번호
```

예

```
Description
- 차량 속도 검증 로직 구현
- 후측방 위험 검증 추가
- fail-safe 정책 적용

Closes #23
```

### PR 작성 원칙

- 하나의 PR은 **하나의 Issue 해결**을 기준으로 작성한다
- PR 본문에는 **무엇을 왜 변경했는지**를 작성한다
- 필요한 경우 테스트 결과, 주의사항, 확인 포인트를 함께 기록한다
- 너무 큰 PR은 지양하고 리뷰 가능한 크기로 나눈다

---

## 8. Code Review

PR Merge 전 반드시 **Code Review를 수행**한다.

### 검토 항목

- 코드 가독성
- 설계 구조
- 버그 여부
- 테스트 여부
- 요구사항 반영 여부

### 리뷰어 지정 및 승인 프로세스

- 모든 PR의 **Reviewers**와 **Assignees**는 팀장이 지정한다
- PR 생성 후 **24시간 이내 리뷰 완료**를 권장한다
- 최소 **1명 이상의 Approve**를 받아야 한다
- 승인 후 작성자 또는 팀장이 최종 확인 후 Merge 한다

### 리뷰 원칙

- 리뷰는 단순 오타보다 **설계, 로직, 안정성 중심**으로 진행한다
- 리뷰 코멘트는 수정 요청인지 제안인지 명확히 구분한다
- 작성자는 리뷰 의견을 반영한 뒤 필요한 답변을 남긴다

---

## 9. Merge 규칙

### Merge 방식

```
Squash Merge
```

### 이유

- commit history 정리
- main branch 기록 간결 유지
- Issue 단위 변경 사항 추적 용이

### Merge 조건

다음 조건을 모두 만족해야 Merge 가능하다.

```
PR 생성 완료
Review 완료
최소 1명 승인
필수 검사 통과
```

### Squash Merge 시 최종 커밋 메시지 규칙

Squash Merge 시 작업 브랜치의 개별 커밋 이력은 하나의 커밋으로 합쳐진다.

따라서 **main branch에 반영되는 최종 커밋 메시지**는 반드시 아래 형식을 따른다.

```
type: PR 제목 (#이슈번호)
```

예

```
feat: add child lock validation logic (#23)
```

### 예시 흐름

```
Branch
feature/23-childlock-validator

Commit
feat: add unlock validation logic
fix: handle rearSideDanger null case

PR title
Add child lock validation logic

PR body
Closes #23

Squash merge commit
feat: add child lock validation logic (#23)
```

---

## 10. Branch 보호 규칙

main branch에는 다음 보호 규칙을 적용한다.

```
direct push 금지
PR 필수
review 필수
필수 검사 통과 후 merge
```

### 권장 설정

- Require a pull request before merging
- Require approvals
- Require status checks to pass before merging
- Restrict direct pushes to main

---

## 11. CI / Actions

CI 및 자동화는 GitHub Actions를 통해 관리한다.

세부 빌드/테스트/검사 규칙은 별도 문서 또는 workflow 설정을 따른다.

기본 원칙은 다음과 같다.

- 필요한 검사 항목은 자동으로 실행한다
- 필수 검사를 통과하지 못하면 merge 하지 않는다

---

## 12. Repository 구조

```
project
 ├─ src
 ├─ docs
 ├─ test
 ├─ .github
 │   └─ workflows
 ├─ README.md
 ├─ GITHUB.md
 ├─ DEVELOPMENT.md
 └─ LICENSE
```

---

## 13. 문서 관리

문서는 다음 위치에 저장한다.

```
docs/
```

예

```
docs
 └─ uml_diagram
```

### 문서 관리 대상

- 프로젝트 개요
- 시스템 구조
- 개발 규칙
- API 명세
- UML 다이어그램
- 회의록 및 결정 사항

---

## 14. 협업 원칙

팀 개발은 다음 원칙을 따른다.

- 모든 작업은 **Issue 기반으로 진행**
- Branch는 **Issue 단위로 생성**
- main 직접 push 금지
- Pull Request 기반 merge
- Code Review 필수
- 필수 검사 통과 후 merge
- 최종 이력은 Squash Merge로 정리

---

## 15. 최종 개발 방식

```
Issue 기반 개발
+
Branch 전략
+
Commit 규칙
+
PR 리뷰
+
Squash Merge 기반 이력 관리
```

이 방식을 통해 **안정적인 협업, 명확한 작업 추적, 코드 품질 유지**를 목표로 한다.
